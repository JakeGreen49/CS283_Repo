#ifndef __DSHLIB_H__
#define __DSHLIB_H__

// Constants for command structure sizes
#define EXE_MAX 64
#define ARG_MAX 256
#define CMD_MAX 8
// Longest command that can be read from the shell
#define SH_CMD_MAX EXE_MAX + ARG_MAX

typedef struct command
{
    char exe[EXE_MAX];
    char args[ARG_MAX];
} command_t;

typedef struct command_list
{
    int num;
    command_t* commands[CMD_MAX];
} command_list_t;

// Special character #defines
#define SPACE_CHAR ' '
#define PIPE_CHAR '|'
#define PIPE_STRING "|"

#define SH_PROMPT "dsh> "
#define EXIT_CMD "exit"
#define DRAGON_CMD "dragon"

// Standard Return Codes
#define OK 0
#define WARN_NO_CMDS -1
#define ERR_TOO_MANY_COMMANDS -2
#define ERR_CMD_OR_ARGS_TOO_BIG -3

// starter code
#define M_NOT_IMPL "The requested operation is not implemented yet!\n"
#define EXIT_NOT_IMPL 3
#define NOT_IMPLEMENTED_YET 0

// prototypes
int make_struct(char* cmd_buff, command_t* command);
char* remove_spaces(char* in);
int build_cmd_list(char *cmd_line, command_list_t *clist);
int print_dragon();

// output constants
#define CMD_OK_HEADER "PARSED COMMAND LINE - TOTAL COMMANDS %d\n"
#define CMD_OK_ARGS "<%d> %s [%s]\n"
#define CMD_OK_NO_ARGS "<%d> %s\n"
#define CMD_WARN_NO_CMD "warning: no commands provided\n"
#define CMD_ERR_PIPE_LIMIT "error: piping limited to %d commands\n"

// dragon command output
#define NUM_DRAGON_LINES 38
#define DRAGON (char*[]) {"72s1a4p23s",	"70s6p24s", "69s6p25s", "65s1p1s7p11s1a14s", \
					"64s10p8s7p11s", "39s7p2s4p1a9s12p1a4s6p2s1a4p8s", "34s22p6s28p10s", \
					"32s26p3s12p1s15p11s", "31s29p1s19p5s3p12s", "29s28p1a1s1a18p8s2p12s", \
					"28s33p1s22p16s", "28s58p14s", "28s50p1a6p1a14s", "6s8p1a11s16p8s26p6s2p16s", \
					"4s13p9s2p1a12p11s11p1s12p6s1a1p16s", "2s10p3s3p8s14p12s24p24s", \
					"1s9p7s1p9s13p13s12p1a11p23s", "9p1a16s1p1s13p12s1a25p21s", "8p1a17s2p1a12p12s1a28p18s", \
					"7p1a19s15p11s33p14s", "10p18s15p10s35p6s4p2s", "9p1a19s1a14p9s12p1a1s4p1s17p3s8p", \
					"10p18s17p8s13p6s18p1s9p", "9p1a2p1a16s16p1a7s14p5s24p2s2p", \
					"1s10p18s1p1s14p1a8s14p3s26p1s2p", "2s12p2s1a11s18p8s40p2s3p1s", \
					"3s13p1s2p2s1p2s1p1a1s18p10s37p4s3p1s", "4s18p1s22p11s1a31p4s7p1s", \
					"5s39p14s28p8s3p3s", "6s1a35p18s25p15s", "8s32p22s19p2s7p10s", \
					"11s26p27s15p2s1a9p9s", "14s20p11s1a1p1a1p18s1a18p3s3p8s", \
					"18s15p8s10p20s15p4s1p9s", "16s36p22s14p12s", "16s26p2s4p1s3p22s10p2s3p1a10s", \
					"21s19p1s6p1s2p26s13p1a10s", "81s7p1a7s"}
#endif
