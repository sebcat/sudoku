#include <stdint.h>
#include <stdio.h>
#include <assert.h>

#define SUDOKU_SET(ctx, x, y, val) \
    ((ctx)->board[(y)*9 + (x)] = 1 << (val))

#define SUDOKU_CLEAR(ctx, x, y, val) \
    ((ctx)->board[(y)*9 + (x)] &= ~(1 << (val)))

#define SUDOKU_ISSET(ctx, x, y, val) \
    ((ctx)->board[(y)*9 + (x)] & (1 << (val)))

#define SUDOKU_NPOSSIBLE(ctx, x, y) \
    __builtin_popcount((ctx)->board[(y)*9 + (x)])

#define ARRAY_SIZE(x) \
  (sizeof((x))/sizeof(*(x)))

struct sudoku_ctx {
  uint16_t board[9*9];
};

void sudoku_init(struct sudoku_ctx *ctx) {
  for (unsigned int i = 0; i < ARRAY_SIZE(ctx->board); i++) {
    ctx->board[i] = 0x1ff;
  }
}

void sudoku_set(struct sudoku_ctx *ctx, unsigned int col, unsigned int row,
    unsigned int val) {
  unsigned int x0;
  unsigned int y0;
  unsigned int x;
  unsigned int y;
  unsigned int i;

  if (col > 8 || row > 8 || val > 8) {
    return;
  }

  /* clear row */
  for (i = 0; i < 9; i++) {
    SUDOKU_CLEAR(ctx, i, row, val);
  }

  /* clear column */
  for (i = 0; i < 9; i++) {
    SUDOKU_CLEAR(ctx, col, i, val);
  }

  /* clear square */
  x0 = (col / 3) * 3;
  y0 = (row / 3) * 3;
  for (y = 0; y < 3; y++) {
    for (x = 0; x < 3; x++) {
      SUDOKU_CLEAR(ctx, x0 + x, y0 + y, val);
    }
  }

  SUDOKU_SET(ctx, col, row, val);
}

void sudoku_load(struct sudoku_ctx *ctx, FILE *in) {
  unsigned int row = 0;
  unsigned int col = 0;
  int ch;

  while ((ch = fgetc(in)) != EOF) {
    if (ch >= '1' && ch <= '9') {
      sudoku_set(ctx, col, row, ch - '1');
      col++;
    } else if (ch == '-') {
      col++;
    } else if (ch == '\n') {
      col = 0;
      row++;
    }
  }
}

static unsigned int check_row(struct sudoku_ctx *ctx, unsigned int row,
    unsigned int val) {
  unsigned int col = 0;
  unsigned int count = 0;

  for (col = 0; col < 9; col++) {
    if (SUDOKU_ISSET(ctx, col, row, val)) {
      count++;
    }
  }

  return count;
}

static unsigned int check_col(struct sudoku_ctx *ctx, unsigned int col,
    unsigned int val) {
  unsigned int row = 0;
  unsigned int count = 0;

  for (row = 0; row < 9; row++) {
    if (SUDOKU_ISSET(ctx, col, row, val)) {
      count++;
    }
  }

  return count;
}

static unsigned int check_square(struct sudoku_ctx *ctx, unsigned int x0,
    unsigned int y0, unsigned int val) {
  unsigned int x;
  unsigned int y;
  unsigned int count = 0;

  for (y = 0; y < 3; y++) {
    for (x = 0; x < 3; x++) {
      if (SUDOKU_ISSET(ctx, x0 + x, y0 + y, val)) {
        count++;
      }
    }
  }

  return count;
}

int sudoku_solve_iter(struct sudoku_ctx *ctx) {
  unsigned int x;
  unsigned int y;
  unsigned int x0;
  unsigned int y0;
  unsigned int val;

  for (y = 0; y < 9; y++) {
    y0 = (y / 3) * 3;
    for (x = 0; x < 9; x++) {
      if (SUDOKU_NPOSSIBLE(ctx, x, y) == 1) {
        continue;
      }

      x0 = (x / 3) * 3;
      for (val = 0; val < 9; val++) {
        if (SUDOKU_ISSET(ctx, x, y, val)) {
          if (check_row(ctx, y, val) == 1 || check_col(ctx, x, val) == 1 ||
              check_square(ctx, x0, y0, val) == 1) {
            sudoku_set(ctx, x, y, val);
            return 1;
          }
        }
      }
    }
  }

  return 0;
}

void sudoku_solve(struct sudoku_ctx *ctx) {
  int i;

  for (i = 0; i < 81; i++) {
    if (!sudoku_solve_iter(ctx)) {
      break;
    }
  }
}

void sudoku_print(struct sudoku_ctx *ctx, FILE *out) {
  unsigned int x;
  unsigned int y;
  unsigned int val;

  for (y = 0; y < 9; y++) {
    for (x = 0; x < 9; x++) {
      for (val = 0; val < 9; val++) {
        if (SUDOKU_ISSET(ctx, x, y, val)) {
          fputc('1' + val, out);
        } else {
          fputc('-', out);
        }
      }

      if (x == 8) {
        if (y == 2 || y == 5) {
          fputc('\n', out);
        }
        fputc('\n', out);
      } else if (x == 2 || x == 5) {
        fprintf(out, "   ");
      } else {
        fputc(' ', out);
      }
    }
  }
}

int main() {
  struct sudoku_ctx ctx;

  sudoku_init(&ctx);
  sudoku_load(&ctx, stdin);
  sudoku_solve(&ctx);
  sudoku_print(&ctx, stdout);
  return 0;
}
