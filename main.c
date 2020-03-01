#ifdef __STDC_ALLOC_LIB__
#define __STDC_WANT_LIB_EXT2__ 1
#else
#define _POSIX_C_SOURCE 200809L
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <getopt.h>
#include <stdint.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <ctype.h>
#include <unistd.h>
#include <json.h>
#include "include/stringop.h"
#include "include/ipc-client.h"

#ifndef SWAYLAY_VERSION
	#define SWAYLAY_VERSION "undefined"
#endif

#include <signal.h>

static volatile sig_atomic_t keep_listening = 1;

char *socket_path = NULL;
int socketfd = 0;

void sig_handler(int _) {
	(void)_;
	keep_listening = 0;
	close(socketfd);
}

static bool success_object(json_object *result) {
	json_object *success;

	if (!json_object_object_get_ex(result, "success", &success)) {
		return true;
	}

	return json_object_get_boolean(success);
}

// Iterate results array and return false if any of them failed
static bool success(json_object *r, bool fallback) {
	if (!json_object_is_type(r, json_type_array)) {
		if (json_object_is_type(r, json_type_object)) {
			return success_object(r);
		}
		return fallback;
	}

	size_t results_len = json_object_array_length(r);
	if (!results_len) {
		return fallback;
	}

	for (size_t i = 0; i < results_len; ++i) {
		json_object *result = json_object_array_get_idx(r, i);

		if (!success_object(result)) {
			return false;
		}
	}

	return true;
}

int main(int argc, char** argv) {
	static bool quiet = false;

	static struct option long_options[] = {
		{"help", no_argument, NULL, 'h'},
		{"quiet", no_argument, NULL, 'q'},
		{"socket", required_argument, NULL, 's'},
		{"version", no_argument, NULL, 'v'},
		{0, 0, 0, 0}
	};

	const char *usage =
		"Usage: swaylay [options] [message]\n"
		"\n"
		"  -h, --help             Show help message and quit.\n"
		"  -q, --quiet            Be quiet.\n"
		"  -s, --socket <socket>  Use the specified socket.\n"
		"  -v, --version          Show the version number and quit.\n";


	int c;
	while (1) {
		int option_index = 0;
		c = getopt_long(argc, argv, "hmpqrs:v", long_options,
				&option_index);
		if (c == -1) {
			break;
		}
		switch (c) {
		case 'q': // Quiet
			quiet = true;
			break;
		case 's': // Socket
			socket_path = strdup(optarg);
			break;
		case 'v':
			fprintf(stdout, "swaymsg version %s\n", SWAYLAY_VERSION);
			exit(EXIT_SUCCESS);
			break;
		default:
			fprintf(stderr, "%s", usage);
			exit(EXIT_FAILURE);
		}
	}

	if (!socket_path) {
		socket_path = get_socketpath();
		if (!socket_path) {
			if (!quiet) {
				printf("Unable to retrieve socket path\n");
			}
			exit(EXIT_FAILURE);
		}
	}

	const uint32_t type = IPC_SUBSCRIBE;

	char *command = NULL;
	if (optind < argc) {
		command = join_args(argv + optind, argc - optind);
	} else {
		command = strdup("");
	}

	int ret = 0;
	socketfd = ipc_open_socket(socket_path);
	struct timeval timeout = {.tv_sec = 3, .tv_usec = 0};
	ipc_set_recv_timeout(socketfd, timeout);
	uint32_t len = strlen(command);
	char *resp = ipc_single_command(socketfd, type, command, &len);

	// print the json
	json_object *obj = json_tokener_parse(resp);
	if (obj == NULL) {
		if (!quiet) {
			fprintf(stderr, "ERROR: Could not parse json response from ipc. "
					"This is a bug in sway.");
			printf("%s\n", resp);
		}
		ret = 1;
	} else {
		if (!success(obj, true)) {
			ret = 2;
		}
		if (!quiet && ret != 0) {
			printf("%s\n", json_object_to_json_string_ext(obj,
				JSON_C_TO_STRING_PRETTY | JSON_C_TO_STRING_SPACED));
		}
		json_object_put(obj);
	}
	free(command);
	free(resp);

	if (ret != 0) {
		printf("Failed to parse JSON object\n");
		exit(1);
	}
	// Remove the timeout for subscribed events
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;
	ipc_set_recv_timeout(socketfd, timeout);

	signal(SIGINT, sig_handler);
	do {
		struct ipc_response *reply = ipc_recv_response(socketfd);
		if (!reply) {
			break;
		}

		json_object *obj = json_tokener_parse(reply->payload);
		if (obj == NULL) {
			if (!quiet) {
				fprintf(stderr, "ERROR: Could not parse json response from"
						" ipc. This is a bug in sway.");
				ret = 1;
			}
			break;
		} else if (quiet) {
			json_object_put(obj);
		} else {
			printf("%s\n", json_object_to_json_string(obj));
			fflush(stdout);
			json_object_put(obj);
		}

		free_ipc_response(reply);
	} while (keep_listening);

	free(socket_path);
	return ret;
}
