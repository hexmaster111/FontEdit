#include <raylib.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

char **ReadAllLines(const char *fname, int *out_linecount);

#define FONT_WIDTH (4)
#define FONT_HEIGHT (8)

int main(int argc, char *argv[])
{
    if (2 > argc)
    {
        printf("Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    int lines_count = 0;
    char **lines = ReadAllLines(argv[1], &lines_count);

    for (size_t i = 0; i < lines_count; i++)
    {
        for (size_t j = 0; j < strlen(lines[i]); j++)
        {
            if (lines[i][j] == '/')
            { // remove comments
                lines[i][j] = '\0';
                break;
            }

            if (lines[i][j] == ' ')
            { // remove spaces
                lines[i][j] = '\0';
                break;
            }
        }

        printf("%s\n", lines[i]);
    }

    unsigned char cur_char = 65;
    int px_size = 8;

    InitWindow(300, 300, "FEDIT");
    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(BLACK);

        #define EDITOR_WIDTH (FONT_WIDTH * px_size)

        for (size_t ed = 0; ed < 8; ed++)
        {
            for (size_t w = 0; w < FONT_WIDTH; w++)
            {
                for (size_t h = 0; h < FONT_HEIGHT; h++)
                {
                    Vector2 pt = {((w * px_size) + ed * EDITOR_WIDTH) + 10, (h * px_size) + 10};
                    Rectangle r = {pt.x, pt.y, px_size, px_size};

                    int cbit = (FONT_HEIGHT - 1 - h) * FONT_WIDTH + (FONT_WIDTH - 1 - w);
                    char ch = lines[cur_char + ed][cbit];
                    bool on = ch == '1';
                    DrawRectangleRec(r, on ? YELLOW : GRAY);

                    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(GetMousePosition(), r))
                    {
                        lines[cur_char + ed][cbit] = on ? '0' : '1';
                    }
                }
            }
        }

        DrawText(TextFormat("%d : '%c'", cur_char, cur_char), 10, 10 + (px_size * FONT_HEIGHT), px_size, WHITE);

        if (IsKeyPressed(KEY_LEFT) || IsKeyPressedRepeat(KEY_LEFT))
        {
            cur_char -= 1;
        }

        if (IsKeyPressed(KEY_RIGHT) || IsKeyPressedRepeat(KEY_RIGHT))
        {
            cur_char += 1;
        }

        Rectangle savepos = {0, 0, px_size * 4, px_size};
        DrawRectangleRec(savepos, WHITE);
        DrawText("SAVE", savepos.x, savepos.y, px_size, BLACK);

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(GetMousePosition(), savepos))
        {
            FILE *f = fopen(argv[1], "w");
            for (size_t i = 0; i < lines_count; i++)
            {
                const char *comment = TextFormat(" // [%d]", i);

                if (isprint(i))
                {
                    comment = TextFormat(" // '%c'", (char)i);
                }

                fprintf(f, "%s%s\n", lines[i], comment);
            }

            fclose(f);
        }

        EndDrawing();
    }

    CloseWindow();
}

char **ReadAllLines(const char *fname, int *out_linecount)
{
    FILE *f = fopen(fname, "r");

    if (f == NULL)
        return NULL;

    int arraylen = 1024;
    int strlen = 1024;

    int lines_used = 0;
    int chars_read = 0;

    char **ret = malloc(sizeof(char *) * arraylen);
    ret[lines_used] = malloc(strlen);

    while (1)
    {

        if (feof(f))
        {
            // we are all done reading, finlise the last line
            ret[lines_used] = realloc(ret[lines_used], chars_read); // shrink string to be size read
            ret[lines_used][chars_read - 1] = 0;                    // terminate the string
            lines_used++;
            break;
        }

        char c = fgetc(f);

        if (ferror(f))
        {
            perror("fgetc");
            return NULL;
        }

        if (chars_read > strlen)
        {
            strlen *= 2;
            ret[lines_used] = realloc(ret[lines_used], strlen);
        }

        if ((c == '\r' || c == '\n') && chars_read > 0)
        {
            // store this line

            ret[lines_used] = realloc(ret[lines_used], chars_read + 1); // shrink string to be size read
            ret[lines_used][chars_read] = 0;                            // terminate the string

            strlen = 1024; // reset var
            chars_read = 0;

            lines_used += 1;

            if (lines_used >= arraylen) // grow the continaing array
            {
                arraylen *= 2;
                ret = realloc(ret, arraylen * sizeof(char *));
            }
            ret[lines_used] = malloc(strlen); // alloc the next string
            continue;
        }
        else if (c == '\r' || c == '\n' && 0 >= chars_read)
        {
            // we just got a stray \r or \n
            continue;
        }

        ret[lines_used][chars_read] = c;
        chars_read += 1;
    }

    // resize the return array to fit used
    ret = realloc(ret, lines_used * sizeof(char *));

    *out_linecount = lines_used;
    return ret;
}