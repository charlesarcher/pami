#include <stdio.h>

#include "cnclassroute.h"

char *dim_names = XMI_DIM_NAMES;

extern void build_node_classroute(rect_t *world, coord_t *worldroot, coord_t *me,
						rect_t *comm, classroute_t *cr);

int sprint_links(char *buf, uint32_t link) {
	char *s = buf;
	int x;
	for (x = 0; x < XMI_MAX_DIMS; ++x) {
		if (link & CR_LINK(x,CR_SIGN_POS)) {
			if (s != buf) *s++ = ' ';
			*s++ = dim_names[x];
			*s++ = '+';
		}
		if (link & CR_LINK(x,CR_SIGN_NEG)) {
			if (s != buf) *s++ = ' ';
			*s++ = dim_names[x];
			*s++ = '-';
		}
	}
	*s = '\0';
	return s - buf;
}

int parse_coord(char *s, char **e, coord_t *c) {
	coord_t c0;
	char *t;

	/* syntax: "( %d, %d, ... )" */
	while (*s && *s != '(') ++s;
	if (*s == '(') ++s;
	c0.dims = 0;
	while (1) {
		while (*s && isspace(*s)) ++s;
		if (!*s) {
			if (e) *e = s;
			return -1;
		}
		if (*s == ')') {
			if (e) *e = ++s;
			*c = c0;
			return 0;
		}
		c0.coords[c0.dims] = strtol(s, &t, 0);
		if (s == t) {
			if (e) *e = s;
			return -1;
		}
		s = t;
		if (*s == ',') ++s;
		++c0.dims;
	}
}

int parse_rect(char *s, char **e, rect_t *r) {
	int x;
	rect_t r0;
	char *t;

	/* syntax: "(%d,%d,...) : (%d,%d,...)" */
	x = parse_coord(s, e, &r0.ll);
	if (x != 0) return x;
	t = *e;
	while (*t && isspace(*t)) ++t;
	if (*t == ':') ++t;
	while (*t && isspace(*t)) ++t;
	x = parse_coord(t, e, &r0.ur);
	if (x != 0) return x;
	if (r0.ll.dims != r0.ur.dims) { *e = s; return -1; }
	*r = r0;
	return 0;
}

int sprint_coord(char *buf, coord_t *co) {
	char *s = buf;
	char c = '(';
	int x;
	for (x = 0; x < co->dims; ++x) {
		s += sprintf(s, "%c%d", c, co->coords[x]);
		c = ',';
	}
	*s++ = ')';
	*s = '\0';
	return s - buf;
}

int sprint_rect(char *buf, rect_t *r) {
	char *s = buf;

	s += sprint_coord(s, &r->ll);
	*s++ = ':';
	s += sprint_coord(s, &r->ur);
	return s - buf;
}

int coord2rank(rect_t *world, coord_t *coord) {
	int rank = 0;
	int d;
	for (d = 0; d < world->ll.dims; ++d) {
		rank *= (world->ur.coords[d] - world->ll.coords[d] + 1);
		rank += coord->coords[d];
	}
	return rank;
}

void rank2coord(rect_t *world, int rank, coord_t *coord) {
	int d;
	for (d = world->ll.dims - 1; d >= 0; --d) {
		int x = (world->ur.coords[d] - world->ll.coords[d] + 1);
		coord->coords[d] = rank % x;
		rank /= x;
	}
	coord->dims = world->ll.dims;
}

int rect_size(rect_t *rect) {
	int size = 1;
	int d;
	for (d = 0; d < rect->ll.dims; ++d) {
		size *= (rect->ur.coords[d] - rect->ll.coords[d] + 1);
	}
	return size;
}

void print_classroute(coord_t *me, classroute_t *cr) {
	static char buf[1024];
	char *s = buf;
	s += sprint_coord(s, me);
	s += sprintf(s, " up: ");
	s += sprint_links(s, cr->up_tree);
	s += sprintf(s, " dn: ");
	s += sprint_links(s, cr->dn_tree);
	printf("%s\n", buf);
}
