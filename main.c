#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE 2048
#define MAX_RULES 100

typedef struct {
    char from[100];
    char to[100];
    int placeholder; // whether rule uses placeholder like INPUT
} Rule;

int load_rules(const char *filename, Rule rules[]) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Error opening rules.conf");
        return 0;
    }

    int count = 0;
    char buf[200];
    while (fgets(buf, sizeof(buf), file) && count < MAX_RULES) {
        char *arrow = strstr(buf, "->");
        if (!arrow) continue;

        *arrow = '\0';
        char *from = buf;
        char *to = arrow + 2;

        // trim whitespace/newlines
        from[strcspn(from, "\r\n")] = 0;
        to[strcspn(to, "\r\n")] = 0;

        strcpy(rules[count].from, from);
        strcpy(rules[count].to, to);
        rules[count].placeholder = (strstr(from, "INPUT") || strstr(to, "INPUT")) ? 1 : 0;

        count++;
    }

    fclose(file);
    return count;
}

void apply_rules(const char *input_line, char *output_line, Rule rules[], int rule_count) {
    strcpy(output_line, input_line);
    for (int i = 0; i < rule_count; i++) {
        if (rules[i].placeholder) {
            // find occurrence of the pattern before placeholder (simplified)
            char *pos = strstr(output_line, rules[i].from);
            while (pos) {
                // find opening parenthesis
                char *open = strchr(pos + strlen(rules[i].from), '(');
                if (!open) break;
                char *close = strchr(open + 1, ')');
                if (!close) break;

                // extract inside
                char inside[500];
                int len = close - (open + 1);
                strncpy(inside, open + 1, len);
                inside[len] = '\0';

                // build replacement
                char rep[600];
                strcpy(rep, rules[i].to);
                // replace "INPUT" in rep with inside
                char *inp = strstr(rep, "INPUT");
                if (inp) {
                    char temp[600];
                    strcpy(temp, inp + 5);
                    *inp = '\0';
                    strcat(rep, inside);
                    strcat(rep, temp);
                }

                // perform replacement in output_line
                char before[2000], after[2000];
                int prefix_len = pos - output_line;
                strncpy(before, output_line, prefix_len);
                before[prefix_len] = '\0';
                strcpy(after, close + 1);

                snprintf(output_line, MAX_LINE, "%s%s%s", before, rep, after);
                pos = strstr(output_line + prefix_len + strlen(rep), rules[i].from);
            }
        } else {
            char *pos = strstr(output_line, rules[i].from);
            while (pos) {
                char temp[2000];
                strcpy(temp, pos + strlen(rules[i].from));
                *pos = '\0';
                strcat(output_line, rules[i].to);
                strcat(output_line, temp);
                pos = strstr(output_line + strlen(rules[i].to), rules[i].from);
            }
        }
    }
}

int main() {
    Rule rules[MAX_RULES];
    int rule_count = load_rules("rules.conf", rules);
    if (rule_count == 0) {
        printf("⚠️ No rules loaded.\n");
        return 1;
    }

    FILE *input = fopen("funcs.lang", "r");
    FILE *output = fopen("output.py", "w");
    if (!input || !output) {
        perror("Error opening funcs.lang or output.py");
        return 1;
    }

    char line[MAX_LINE], processed[MAX_LINE];
    while (fgets(line, sizeof(line), input)) {
        apply_rules(line, processed, rules, rule_count);
        fputs(processed, output);
    }

    fclose(input);
    fclose(output);

    printf("✅ Converted code written to output.py\n");
    system("python3 output.py");
    return 0;
}
