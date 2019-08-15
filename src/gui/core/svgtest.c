#include"svg.h"

int main(int argc, const char*argv[]) {
	svg_t *svg;
	svg_create(&svg);
	svg_status_t rc = svg_trace_render_engine(svg);
	rc = svg_parse(svg, (argc>1?argv[1]:"/mnt/nfs/alitest/button.svg"));
	//rc = svg_parse_buffer(ctx->svg, buffer, buffer_size);
}

