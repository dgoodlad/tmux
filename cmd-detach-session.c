/* $Id$ */

/*
 * Copyright (c) 2007 Nicholas Marriott <nicm@users.sourceforge.net>
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

#include "tmux.h"

/*
 * Detach session. If called with -a detach all clients attached to specified
 * session, otherwise detach current session on key press only.
 */

int		 cmd_detach_session_parse(void **, int, char **, char **);
const char	*cmd_detach_session_usage(void);
void		 cmd_detach_session_exec(void *, struct cmd_ctx *);
void		 cmd_detach_session_send(void *, struct buffer *);
void		 cmd_detach_session_recv(void **, struct buffer *);
void		 cmd_detach_session_free(void *);

struct cmd_detach_session_data {
	int	 flag_all;
};

const struct cmd_entry cmd_detach_session_entry = {
	CMD_DETACHSESSION, "detach-session", "detach", 0,
	cmd_detach_session_parse,
	cmd_detach_session_usage,
	cmd_detach_session_exec,
	cmd_detach_session_send,
	cmd_detach_session_recv,
	cmd_detach_session_free
};

int
cmd_detach_session_parse(void **ptr, int argc, char **argv, char **cause)
{
	struct cmd_detach_session_data	*data;
	int				 opt;

	*ptr = data = xmalloc(sizeof *data);
	data->flag_all = 0;

	while ((opt = getopt(argc, argv, "a")) != EOF) {
		switch (opt) {
		case 'a':
			data->flag_all = 1;
			break;
		default:
			goto usage;
		}
	}	
	argc -= optind;
	argv += optind;
	if (argc != 0)
		goto usage;

	return (0);

usage:
	usage(cause, "%s", cmd_detach_session_usage());

	xfree(data);
	return (-1);
}

const char *
cmd_detach_session_usage(void)
{
	return ("detach-session [-a]");
}

void
cmd_detach_session_exec(void *ptr, struct cmd_ctx *ctx)
{
	struct cmd_detach_session_data	*data = ptr, std = { 0 };
	struct client			*c = ctx->client, *cp;
	struct session			*s = ctx->session;
	u_int				 i;

	if (data == NULL)
		data = &std;

	if (data->flag_all) {
		for (i = 0; i < ARRAY_LENGTH(&clients); i++) {
			cp = ARRAY_ITEM(&clients, i);
			if (cp == NULL || cp->session != s)
				continue;
			server_write_client(cp, MSG_DETACH, NULL, 0);
		}
	} else if (ctx->flags & CMD_KEY)
		server_write_client(c, MSG_DETACH, NULL, 0);

	if (!(ctx->flags & CMD_KEY))
		server_write_client(c, MSG_EXIT, NULL, 0);
}

void
cmd_detach_session_send(void *ptr, struct buffer *b)
{
	struct cmd_detach_session_data	*data = ptr;

	buffer_write(b, data, sizeof *data);
}

void
cmd_detach_session_recv(void **ptr, struct buffer *b)
{
	struct cmd_detach_session_data	*data;

	*ptr = data = xmalloc(sizeof *data);
	buffer_read(b, data, sizeof *data);
}

void
cmd_detach_session_free(void *ptr)
{
	struct cmd_detach_session_data	*data = ptr;

	xfree(data);
}
