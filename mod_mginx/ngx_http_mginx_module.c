/*
 * Copyright (C) ivan
 */

#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>
#include "gtmxc_types.h"

static char* ngx_http_mginx(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);

static void* ngx_http_mginx_create_loc_conf(ngx_conf_t *cf);

static char* ngx_http_mginx_merge_loc_conf(ngx_conf_t *cf,
    void *parent, void *child);

typedef struct {
    u_int		 loc_len;
    ngx_flag_t           enable;
} ngx_http_mginx_loc_conf_t;

static ngx_int_t ngx_http_mginx_init(ngx_http_mginx_loc_conf_t *cf);

static ngx_int_t ngx_http_mginx_init_process(ngx_cycle_t *cl);
void ngx_http_mginx_exit_process(ngx_cycle_t *cl);

static ngx_command_t  ngx_http_mginx_commands[] = {
    { ngx_string("mginx"),
      NGX_HTTP_LOC_CONF|NGX_CONF_NOARGS,
      ngx_http_mginx,
      NGX_HTTP_LOC_CONF_OFFSET,
      0,
      NULL },

      ngx_null_command
};


static ngx_http_module_t  ngx_http_mginx_module_ctx = {
    NULL,                          /* preconfiguration */
    NULL,			   /* postconfiguration */

    NULL,                          /* create main configuration */
    NULL,                          /* init main configuration */

    NULL,                          /* create server configuration */
    NULL,                          /* merge server configuration */

    ngx_http_mginx_create_loc_conf,/* create location configuration */
    ngx_http_mginx_merge_loc_conf  /* merge location configuration */
};


ngx_module_t  ngx_http_mginx_module = {
    NGX_MODULE_V1,
    &ngx_http_mginx_module_ctx,	   /* module context */
    ngx_http_mginx_commands,	   /* module directives */
    NGX_HTTP_MODULE,               /* module type */
    NULL,                          /* init master */
    NULL,                          /* init module */
    ngx_http_mginx_init_process,   /* init process */
    NULL,                          /* init thread */
    NULL,                          /* exit thread */
    ngx_http_mginx_exit_process,   /* exit process */
    NULL,                          /* exit master */
    NGX_MODULE_V1_PADDING
};


static ngx_int_t
ngx_http_mginx_handler(ngx_http_request_t *r)
{
    ngx_int_t     rc;
    gtm_long_t	  gld;
    ngx_chain_t   out;
    gtm_status_t  gtm_status; 
    gtm_char_t    gtm_buf[2048];

    if (!(r->method & (NGX_HTTP_GET|NGX_HTTP_HEAD))) {
        return NGX_HTTP_NOT_ALLOWED;
    }

    rc = ngx_http_discard_request_body(r);

    if (rc != NGX_OK && rc != NGX_AGAIN) {
        return rc;
    }

    ngx_http_mginx_loc_conf_t  *cglcf;
    cglcf = ngx_http_get_module_loc_conf(r, ngx_http_mginx_module);
    r->headers_out.status = NGX_HTTP_OK;

    gld = atol((const char*) r->args.data);

    if (gld)
    {
	gtm_status = gtm_ci("setData", (gtm_long_t) gld);
	if (gtm_status)
	{
	    gtm_zstatus(gtm_buf, 2048);
	    ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, gtm_buf);
	    r->headers_out.status = NGX_HTTP_INTERNAL_SERVER_ERROR;
	}
    }

    r->headers_out.content_type.len = sizeof("plain/text") - 1;
    r->headers_out.content_type.data = (u_char *) "plain/text";

    r->headers_out.content_length_n = 0;

    r->header_only = 1;

    if (r->method == NGX_HTTP_HEAD) {
        rc = ngx_http_send_header(r);

        if (rc == NGX_ERROR || rc > NGX_OK || r->header_only) {
            return rc;
        }
    }

    out.buf = NULL;
    out.next = NULL;

    rc = ngx_http_send_header(r);

    if (rc == NGX_ERROR || rc > NGX_OK || r->header_only) {
        return rc;
    }

    return ngx_http_output_filter(r, &out);
}

static char *
ngx_http_mginx(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_http_core_loc_conf_t  *clcf;
    ngx_http_mginx_loc_conf_t *cglcf = conf;

    clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);
    clcf->handler = ngx_http_mginx_handler;

    cglcf->loc_len = clcf->name.len;
    cglcf->enable = 1;

    return NGX_CONF_OK;
}

static ngx_int_t
ngx_http_mginx_init(ngx_http_mginx_loc_conf_t *cglcf)
{
  return NGX_OK;
}

static ngx_int_t
ngx_http_mginx_init_process(ngx_cycle_t *cl)
{
  gtm_status_t gtm_status;

  gtm_status = gtm_init();
  if (gtm_status)
	return NGX_ERROR;

  gtm_ci("init");

  return NGX_OK;
}

void
ngx_http_mginx_exit_process(ngx_cycle_t *cl)
{
  gtm_status_t gtm_status;

  gtm_status = gtm_exit();

  return;
}

static void *
ngx_http_mginx_create_loc_conf(ngx_conf_t *cf)
{
    ngx_http_mginx_loc_conf_t  *conf;

    conf = ngx_pcalloc(cf->pool, sizeof(ngx_http_mginx_loc_conf_t));
    if (conf == NULL) {
        return NGX_CONF_ERROR;
    }
    conf->enable = NGX_CONF_UNSET;
    return conf;
}

static char *
ngx_http_mginx_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child)
{
    ngx_http_mginx_loc_conf_t *conf = child;

    if(conf->enable)
        ngx_http_mginx_init(conf);

    return NGX_CONF_OK;
}
