/* $Id$ */

/*
 * Copyright (c) 2008 Nicholas Marriott <nicm@users.sourceforge.net>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF MIND, USE, DATA OR PROFITS, WHETHER
 * IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING
 * OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <sys/types.h>

#include <getopt.h>
#include <stdlib.h>

#include "tmux.h"

/*
 * Change session name.
 */

int	cmd_send_keys_parse(void **, int, char **, char **);
void	cmd_send_keys_exec(void *, struct cmd_ctx *);
void	cmd_send_keys_send(void *, struct buffer *);
void	cmd_send_keys_recv(void **, struct buffer *);
void	cmd_send_keys_free(void *);

struct cmd_send_keys_data {
  	u_int	 nkeys;
	int	*keys;
};

const struct cmd_entry cmd_send_keys_entry = {
	"send-keys", "send", "key ...",
	0,
	cmd_send_keys_parse,
	cmd_send_keys_exec,
	cmd_send_keys_send,
	cmd_send_keys_recv,
	cmd_send_keys_free
};

int
cmd_send_keys_parse(void **ptr, int argc, char **argv, char **cause)
{
	struct cmd_send_keys_data	*data;
	int				 opt, key;
	char				*s;

	*ptr = data = xmalloc(sizeof *data);
	data->nkeys = 0;
	data->keys = NULL;

	while ((opt = getopt(argc, argv, "")) != EOF) {
		switch (opt) {
		default:
			goto usage;
		}
	}
	argc -= optind;
	argv += optind;
	if (argc == 0)
		goto usage;

	while (argc-- != 0) {
		if ((key = key_string_lookup_string(*argv)) != KEYC_NONE) {
			data->keys = xrealloc(
			    data->keys, data->nkeys + 1, sizeof *data->keys);
			data->keys[data->nkeys++] = key;
		} else {
			for (s = *argv; *s != '\0'; s++) {
				data->keys = xrealloc(data->keys,
				    data->nkeys + 1, sizeof *data->keys);
				data->keys[data->nkeys++] = *s;
			}
		}

		argv++;
	}

	return (0);

usage:
	usage(cause, "%s %s",
	    cmd_send_keys_entry.name, cmd_send_keys_entry.usage);

	cmd_send_keys_free(data);
	return (-1);
}

void
cmd_send_keys_exec(void *ptr, struct cmd_ctx *ctx)
{
	struct cmd_send_keys_data	*data = ptr;
	u_int				 i;

	if (data == NULL)
		return;

	for (i = 0; i < data->nkeys; i++)
		window_key(ctx->client->session->curw->window, data->keys[i]);

	if (ctx->cmdclient != NULL)
		server_write_client(ctx->cmdclient, MSG_EXIT, NULL, 0);
}

void
cmd_send_keys_send(void *ptr, struct buffer *b)
{
	struct cmd_send_keys_data	*data = ptr;

	buffer_write(b, data, sizeof *data);
	buffer_write(b, data->keys, data->nkeys * sizeof *data->keys);
}

void
cmd_send_keys_recv(void **ptr, struct buffer *b)
{
	struct cmd_send_keys_data	*data;

	*ptr = data = xmalloc(sizeof *data);
	buffer_read(b, data, sizeof *data);
	data->keys = xcalloc(data->nkeys, sizeof *data->keys);
	buffer_read(b, data->keys, data->nkeys * sizeof *data->keys);
}

void
cmd_send_keys_free(void *ptr)
{
	struct cmd_send_keys_data	*data = ptr;

	if (data->keys != NULL)
		xfree(data->keys);
	xfree(data);
}