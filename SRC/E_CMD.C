#include "E.H"
#include "E_CMD.H"
#include "E_FONT.H"
#include "E_KEYBD.H"
#include "E_MAP.H"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define CMD_BUFFER_LEN ((320 / 8) + 1)

// static uint8 isInteger(const char* str);
// static uint8 isFloat(const char* str);
static void inputCommand();
static void parseCommand(int argc, char** argv);



/**
 * All credits of cmd2argv go to https://github.com/pasztorpisti/cmd2argv
 */
typedef enum {
	C2A_OK,
	C2A_BUF_TOO_SMALL,	// *buf_len isn't large enough
	C2A_ARGV_TOO_SMALL,	// *argv_len isn't large enough
	C2A_UNMATCHED_QUOT,	// reached the end of cmd while looking for matching ' or "
	C2A_LONELY_ESCAPE,	// cmd ends with \ but the escaped character is missing
	C2A_MALLOC,			// malloc() failed. returned only by cmd2argv_malloc().
						// cmd2argv() never returns this.
	C2A_COUNT,			// the number of possible return values (not an actual return value)
} C2A_RESULT;

#define ONLY_COUNTING_BUF_LEN (!buf)

C2A_RESULT
cmd2argv(const char* cmd, char* buf, size_t* buf_len,
		char** argv, size_t* argv_len, size_t* err_pos) {
	size_t argc = 0;
	const char* p = cmd;
	const char* last_quot_pos = NULL;
	char* dest = buf;
	char* dest_end = ONLY_COUNTING_BUF_LEN ? NULL : (dest + *buf_len);
	char weak_quoting = 0;

	// argv has to have space at least for the terminating NULL
	if (!ONLY_COUNTING_BUF_LEN && *argv_len < 1)
		return C2A_ARGV_TOO_SMALL;

	while (*p) {
		while (*p == ' ' || *p == '\t' || *p == '\n')
			p++;
		if (!*p)
			break;

		if (!ONLY_COUNTING_BUF_LEN) {
			// It is +2 because the buffer has to be able to hold at least
			// another argv pointer plus a closing NULL argv pointer.
			if (*argv_len < argc + 2)
				return C2A_ARGV_TOO_SMALL;
			argv[argc] = dest;
		}
		argc++;

		while (*p) {
			switch (*p) {
			case '"':
				last_quot_pos = p;
				weak_quoting ^= 1;
				p++;
				break;

			case '\\':
				if (!p[1]) {
					if (err_pos)
						*err_pos = (size_t)(p - cmd);
					return C2A_LONELY_ESCAPE;
				}
				if (!ONLY_COUNTING_BUF_LEN) {
					if (dest >= dest_end)
						return C2A_BUF_TOO_SMALL;
					*dest = p[1];
				}
				dest++;
				p += 2;
				break;

			case ' ':
			case '\t':
			case '\n':
				if (weak_quoting)
					goto default_handling;
				p++;
				// double break from both the switch and the while loop
				goto end_arg;

			case '\'':
				if (weak_quoting)
					goto default_handling;
				// strong quoting
				last_quot_pos = p;
				if (ONLY_COUNTING_BUF_LEN) {
					for (p=p+1; *p && *p != '\''; dest++,p++) {}
				} else {
					for (p=p+1; *p && *p != '\'' && dest < dest_end; dest++,p++)
						*dest = *p;
					if (dest >= dest_end)
						return C2A_BUF_TOO_SMALL;
				}
				if (!*p) {
					if (err_pos)
						*err_pos = (size_t)(last_quot_pos - cmd);
					return C2A_UNMATCHED_QUOT;
				}
				p++;
				break;

default_handling:
			default:
				if (!ONLY_COUNTING_BUF_LEN) {
					if (dest >= dest_end)
						return C2A_BUF_TOO_SMALL;
					*dest = *p;
				}
				dest++;
				p++;
				break;
			}
		}
		if (weak_quoting) {
			if (err_pos)
				*err_pos = (size_t)(last_quot_pos - cmd);
			return C2A_UNMATCHED_QUOT;
		}
end_arg:
		if (!ONLY_COUNTING_BUF_LEN) {
			if (dest >= dest_end)
				return C2A_BUF_TOO_SMALL;
			*dest = 0;
		}
		dest++;
	}

	if (!ONLY_COUNTING_BUF_LEN)
		argv[argc] = NULL;
	if (argv_len)
		*argv_len = argc + 1;
	if (buf_len)
		*buf_len = dest - buf;
	return C2A_OK;
}

C2A_RESULT
cmd2argv_malloc(const char* cmd, char*** argv, size_t* argv_len, size_t* err_pos) {
	size_t buf_len, _argv_len, argv_size;
	char* buf;

	// 1st pass: querying the required buffer size
	C2A_RESULT res = cmd2argv(cmd, NULL, &buf_len, NULL, &_argv_len, err_pos);
	if (res != C2A_OK)
		return res;

	// allocating the buffer
	argv_size = _argv_len * sizeof(char*);
	buf = malloc(argv_size + buf_len);
	if (!buf)
		return C2A_MALLOC;

	// 2nd pass: splitting cmd into the allocated buffer
	res = cmd2argv(cmd, buf+argv_size, &buf_len, (char**)buf, &_argv_len, NULL);
	// At this point cmd2argv() must succeed. A return value other
	// than C2A_OK suggests a bug in cmd2argv().
	// assert(res == C2A_OK);
	*argv = (char**)buf;
	if (argv_len)
		*argv_len = _argv_len;
	return res;
}

void
cmd2argv_free(char** argv) {
	free(argv);
}



// static uint8 isInteger(const char* str) {
//     char* c = str;
//     while (*c != '\0') {
//         if (*c < '0' || *c > '9')
//             return FALSE;
//     }
//     return TRUE;
// }

// static uint8 isFloat(const char* str);

static void parseCommand(int argc, char** argv) {
    int iValue;
    float fValue;

    // Assignment
    if (argc == 3 && !strcmp("=", argv[1])) {

        if (!strcmp(argv[0], "g_MapFogDist")) {
            if (sscanf(argv[2], "%f", &fValue) == 1) {
                g_MapFogDist = fValue;
                return;
            }

        } else if (!strcmp(argv[0], "g_MapWidth")) {
            if (sscanf(argv[2], "%d", &iValue) == 1 && iValue > 2) {
                MapManager_Resize((uint16)iValue, g_MapHeight);
                return;
            }

        } else if (!strcmp(argv[0], "g_MapHeight")) {
            if (sscanf(argv[2], "%d", &iValue) == 1 && iValue > 2) {
                MapManager_Resize(g_MapWidth, (uint16)iValue);
                return;
            }

        }

        Log("Invalid assignment.");
        return;
    }

}

static void inputCommand() {
    char command[CMD_BUFFER_LEN];
	char buf[CMD_BUFFER_LEN];
	size_t bufLen = CMD_BUFFER_LEN;
	char* argv[4];
	size_t argc = 4;
	Keyboard_InputString(2, 190, 9, command, sizeof(command));

    if (cmd2argv(command, buf, &bufLen, argv, &argc, NULL) != C2A_OK) {
		Log("Invalid command.");
        return;
	}

    if (--argc > 0) {
        parseCommand(argc, argv);
    }

    // Make sure we won't open the editor when pressing enter
    if (Keyboard_Read(Key_Enter) == PRESSED) {
        Keyboard_Clear(Key_Enter);
    }
}

void CMD_Update() {
    if (Keyboard_Read(Key_Slash) != PRESSED)
        return;
    inputCommand();
}




