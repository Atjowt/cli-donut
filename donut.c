#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

static const double TAU = 6.283185307179586;

double lerp(double a, double b, double t) {
	return a + t * (b - a);
}

double frand(void) {
	return rand() / (double)RAND_MAX;
}

typedef struct {
	double x, y, z;
} vec3;

vec3 vec3_add(vec3 u, vec3 v) {
	return (vec3) { u.x + v.x, u.y + v.y, u.z + v.z };
}

vec3 vec3_sub(vec3 u, vec3 v) {
	return (vec3) { u.x - v.x, u.y - v.y, u.z - v.z };
}

vec3 vec3_scale(vec3 v, double s) {
	return (vec3) { v.x * s, v.y * s, v.z * s };
}

vec3 vec3_prod(vec3 u, vec3 v) {
	return (vec3) { u.x * v.x, u.y * v.y, u.z * v.z };
}

vec3 vec3_lerp(vec3 a, vec3 b, double t) {
	return vec3_add(a, vec3_scale(vec3_sub(b, a), t));
}

vec3 vec3_min(vec3 u, vec3 v) {
	return (vec3) { fmin(u.x, v.x), fmin(u.y, v.y), fmin(u.z, v.z) };
}

vec3 vec3_max(vec3 u, vec3 v) {
	return (vec3) { fmax(u.x, v.x), fmax(u.y, v.y), fmax(u.z, v.z) };
}
vec3 vec3_clamp(vec3 v, vec3 a, vec3 b) {
	return vec3_min(vec3_max(v, a), b);
}

double vec3_dot(vec3 u, vec3 v) {
	return u.x * v.x + u.y * v.y + u.z * v.z;
}

vec3 vec3_cross(vec3 u, vec3 v) {
	return (vec3) {
		u.y * v.z - u.z * v.y,
		-(u.x * v.z - u.z * v.x),
		u.x * v.y - u.y * v.x,
	};
}

double vec3_lensqr(vec3 v) {
	return vec3_dot(v, v);
}

double vec3_len(vec3 v) {
	return sqrt(vec3_lensqr(v));
}

double vec3_distsqr(vec3 a, vec3 b) {
	return vec3_lensqr(vec3_sub(b, a));
}

double vec3_dist(vec3 a, vec3 b) {
	return sqrt(vec3_distsqr(a, b));
}

vec3 vec3_normalize(vec3 v) {
	return vec3_scale(v, 1.0 / vec3_len(v));
}

vec3 vec3_from_angles(double theta, double phi) {
	double sin_phi = sin(phi);
	double cos_phi = cos(phi);
	double sin_theta = sin(theta);
	double cos_theta = cos(theta);
	return (vec3) {
		cos_phi * cos_theta,
		cos_phi * sin_theta,
		sin_phi,
	};
}

vec3 vec3_random(void) {
	double theta = TAU * frand();
	double phi = 0.5 * TAU * frand();
	return vec3_from_angles(theta, phi);
}

vec3 vec3_reflect(vec3 v, vec3 n) {
	return vec3_sub(v, vec3_scale(n, 2.0 * vec3_dot(v, n)));
}

vec3 vec3_rotate_x(vec3 v, double t) {
	double cos_t = cos(t);
	double sin_t = sin(t);
	return (vec3) {
		.x = v.x,
		.y = v.y * cos_t - v.z * sin_t,
		.z = v.y * sin_t + v.z * cos_t
	};
}

vec3 vec3_rotate_y(vec3 v, double t) {
	double cos_t = cos(t);
	double sin_t = sin(t);
	return (vec3) {
		.x = v.x * cos_t + v.z * sin_t,
		.y = v.y,
		.z = v.z * cos_t - v.x * sin_t
	};
}

static double sd_torus(vec3 p, double a, double b) {
	double x = vec3_len((vec3) { p.x, 0.0, p.z }) - a;
	vec3 q = { x, p.y, 0.0 };
	return vec3_len(q) - b;
}

static double donut_angle = 0.0;

static double sdf(vec3 p) {
	p = vec3_rotate_y(p, donut_angle * 1.0);
	p = vec3_rotate_x(p, TAU * 0.125);
	return sd_torus(p, 0.4, 0.2);
}

static vec3 normal(vec3 p) {
	static const double DELTA = 0.001;
	vec3 dx = { DELTA, 0.0, 0.0 };
	vec3 dy = { 0.0, DELTA, 0.0 };
	vec3 dz = { 0.0, 0.0, DELTA };
	return vec3_normalize((vec3) {
		.x = sdf(vec3_add(p, dx)) - sdf(p),
		.y = sdf(vec3_add(p, dy)) - sdf(p),
		.z = sdf(vec3_add(p, dz)) - sdf(p),
	});
}

static double shade(vec3 rd, vec3 n) {
	static const double R = 0.577350269;
	static const vec3 SUN = (vec3) { -R, -R, R };
	static const double SOFT = 0.1;
	vec3 h = vec3_normalize(vec3_add(vec3_scale(rd, -1.0), vec3_scale(SUN, -1.0)));
	double diffuse = 0.5 * fmax(SOFT, -vec3_dot(n, SUN));
	double specular = 2.0 * pow(vec3_dot(n, h), 32.0);
	return diffuse + specular;
}

static double march(vec3 ro, vec3 rd) {
	static const int MAX_STEPS = 32;
	static const double MAX_DIST = 32.0;
	static const double MIN_DIST = 0.001;
	double t = 0.0;
	for (int i = 0; i < MAX_STEPS; i++) {
		if (t > MAX_DIST) break;
		vec3 p = vec3_add(ro, vec3_scale(rd, t));
		double d = sdf(p);
		if (d < MIN_DIST) {
			vec3 n = normal(p);
			return shade(rd, n);
			/* return 1.0 - (double)i / MAX_STEPS; */
		}
		t += d;
	}
	return 0.0;
}

static char quantize(double v) {
	if (v <= 0.0) return ' ';
	if (v <= 0.1) return '.';
	if (v <= 0.2) return '+';
	if (v <= 0.4) return '*';
	if (v <= 0.8) return '#';
	return '@';
}

void sleep(clock_t t) {
	clock_t t0 = clock();
	while (clock() - t0 < t);

}

int main(void) {
	static const int IMG_WIDTH = 32, IMG_HEIGHT = 32;
	static const vec3 ro = { 0.0, 0.0, -1.0 };
	for (;;) {
		printf("\x1b[2J\x1b[H");
		for (int y = 0; y < IMG_WIDTH; y++) {
			double ny = -(2.0 * y / (double)(IMG_HEIGHT - 1) - 1.0);
			for (int x = 0; x < IMG_WIDTH; x++) {
				double nx = 2.0 * x / (double)(IMG_WIDTH - 1) - 1.0;
				vec3 rd = vec3_normalize((vec3) { nx, ny, 1.0 });
				double v = march(ro, rd);
				char c = quantize(v);
				putchar(c);
				putchar(' ');
			}
			putchar('\n');
		}
		donut_angle += 0.01 * TAU;
		sleep(30000);
	}
	return 0;
}


