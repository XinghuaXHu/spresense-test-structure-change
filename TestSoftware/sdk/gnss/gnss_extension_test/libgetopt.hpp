#ifndef _LIBGETOPTXXD_H
#define _LIBGETOPTXXD_H 1

#ifdef  __cplusplus
extern "C" {
#endif

extern char *optarg;
extern int optind;
extern int optopt;

struct option
{
    const char *name;
    int has_arg;
    int *flag;
    int val;
};

/* Names for the values of the `has_arg' field of `struct option'.  */
#define no_argument     0
#define required_argument   1
#define optional_argument   2

extern int parse_arg_opt_long (int argc, char *const *argv, const char *shortopts,
                               const struct option *longopts, int *longind);
extern int parse_arg_opt_long_only (int argc, char *const *argv,
                                    const char *shortopts,
                                    const struct option *longopts, int *longind);
#ifdef  __cplusplus
}
#endif

#endif /* libgetopt.h */
