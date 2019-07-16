#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <argp.h>
#include <unistd.h>


void indent(FILE *fp, int i);
void start_compilation(FILE *fp, char *code, int verbose, int tapesize);
int compile_code(FILE *fp, char *code, int ind, int verbose);
char *read_to_EOF(FILE* fp, size_t size);


const char *argp_program_version = "CBFC 1.0";

struct arguments {
    char *args[1];
    char *output;
    int verbose;
    int tapesize;
    int remove;
    int tcc;
};

static struct argp_option options[] =
{
    {"output", 'o', "OUTPUT", 0, "The output file (default: a.c and a.out)"},
    {"verbose", 'v', 0, 0, "Produce verbose output"},
    {"tapesize", 's', "SIZE", 0, "Size of the tape (default: 50000)"},
    {"remove", 'r', 0, 0, "Remove the c source file after compilation"},
    {"tcc", 't', 0, 0, "Compile with tcc instead of gcc (might be necessary for some large files, but execution time might be slower)"},
    {0}
};

static error_t parse_opt(int key, char *arg, struct argp_state *state) {
    struct arguments *arguments = state->input;

    switch (key) {
        case 'v':
            arguments->verbose = 1;
            break;
        case 's':
            arguments->tapesize = atoi(arg);
            break;
        case 'o':
            arguments->output = arg;
            break;
        case 'r':
            arguments->remove = 1;
            break;
        case 't':
            arguments->tcc = 1;
            break;
        case ARGP_KEY_ARG:
            if (state->arg_num >= 1) {
                argp_usage(state);
            }
            arguments->args[state->arg_num] = arg;
            break;
        case ARGP_KEY_END:
            if (state->arg_num < 1) {
                argp_usage(state);
            }
            break;
        default:
            return ARGP_ERR_UNKNOWN;
    }
    
    return 0;
}

static char args_doc[] = "input";

static char doc[] = "CBFC -- C BrainFuck Compiler. Compiles BrainFuck code to a decently optimized c source file, then compiles that source file.";

static struct argp argp = {options, parse_opt, args_doc, doc};

int main(int argc, char **argv) {
    FILE *fp_input, *fp_output;
    char *src_file_name;
    struct arguments arguments;
    char *code;
    char *compile_command;

    /* Setting default args */
    arguments.verbose = 0;
    arguments.tapesize = 50000;
    arguments.output = "a.out";
    arguments.remove = 0;
    arguments.tcc = 0;

    /* parse arguments */
    argp_parse(&argp, argc, argv, 0, 0, &arguments);

    /* Read the code from the file */
    fp_input = fopen(arguments.args[0], "r");
    code = read_to_EOF(fp_input, 10);
    fclose(fp_input);

    /* Compile the file to c*/
    src_file_name = malloc(sizeof(char) * (strlen(arguments.output) + 3));
    strcpy(src_file_name, arguments.output);
    strcat(src_file_name, ".c");
    fp_output = fopen(src_file_name, "w");

    start_compilation(fp_output, code, arguments.verbose, arguments.tapesize);

    fclose(fp_output);

    /* Compile c source */
    compile_command = malloc(sizeof(char) * (9 + strlen(src_file_name) + strlen(arguments.output)));

    if (arguments.tcc)
        sprintf(compile_command, "tcc %s -o %s", src_file_name, arguments.output);
    else
        sprintf(compile_command, "gcc %s -o %s", src_file_name, arguments.output);

    system(compile_command);

    if (arguments.remove) remove(src_file_name);

    /* Free stuff */
    free(src_file_name);
    free(compile_command);
    return 0;
}

void indent(FILE *fp, int i) {
    int j;
    for (j = 0; j < i; j++)
        fprintf(fp, "    ");
}


void start_compilation(FILE *fp, char *code, int verbose, int tapesize) {
    if (verbose) printf("Starting compilation...\n");
    fprintf(fp, "#include <stdio.h>\n#include <stdlib.h>\n");
    fprintf(fp, "#define tape_size %d\n", tapesize);
    fprintf(fp, "int main() {\n");
    indent(fp, 1);
    fprintf(fp, "unsigned char *tape=malloc(tape_size*sizeof(char));\n");
    indent(fp, 1);
    fprintf(fp, "char c;\n");
    compile_code(fp, code, 1, verbose);
    indent(fp, 1);
    fprintf(fp, "free(tape);\n");
    indent(fp, 1);
    fprintf(fp, "return 0;\n");
    fprintf(fp, "}\n");
    if (verbose) printf("Finished compilation.\n");
}

int compile_code(FILE *fp, char *code, int ind, int verbose) {
    /* Add required code */
    int len, pc, counter;
    int i, j, break_flag = 0;
    char c;
    len = strlen(code);

    if (verbose) printf("Depth: %d\nCode length: %d\n\n", ind, len);

    for (i = 0; i < len; i++) {
        c = *(code+i);
        switch (c) {
            case '+':
                indent(fp, ind);
                for (j = 0; *(code+i+j) == '+'; j++);
                fprintf(fp, "(*tape)+=%d;\n", j);
                i += j - 1;
                break;
            case '-':
                indent(fp, ind);
                for (j = 0; *(code+i+j) == '-'; j++);
                fprintf(fp, "(*tape)-=%d;\n", j);
                i += j - 1;
                break;
            case '>':
                indent(fp, ind);
                for (j = 0; *(code+i+j) == '>'; j++);
                fprintf(fp, "tape+=%d;\n", j);
                i += j - 1;
                break;
            case '<':
                indent(fp, ind);
                for (j = 0; *(code+i+j) == '<'; j++);
                fprintf(fp, "tape-=%d;\n", j);
                i += j - 1;
                break;
            case '.':
                indent(fp, ind);
                fprintf(fp, "printf(\"%%c\", *tape);\n");
                break;
            case ',':
                indent(fp, ind);
                fprintf(fp, "if ((c=getchar()) != EOF) *tape = c;\n");
                break;
            case '[':
                indent(fp, ind);
                fprintf(fp, "while(*tape) {\n");
                i += compile_code(fp, code+i+1, ind+1, verbose) + 1;
                indent(fp, ind);
                fprintf(fp, "}\n");
                break;
            case ']':
                break_flag = 1;
                break;
        }
        if (break_flag) break;
    }
    return i;
}


/* Shamelessly taken from stack overflow */
char *read_to_EOF(FILE* fp, size_t size){
    char *str;
    int ch;
    size_t len = 0;
    str = realloc(NULL, sizeof(char)*size);
    if (!str) return str;
    while (EOF != (ch = fgetc(fp))) {
        str[len++] = ch;
        if (len == size) {
            str = realloc(str, sizeof(char)*(size+=16));
            if (!str) return str;
        }
    }
    str[len++] = '\0';

    return realloc(str, sizeof(char)*len);
}
