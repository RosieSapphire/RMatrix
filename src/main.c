#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/ioctl.h>

#include "rosepetal.h"

#define REFRESH_RATE 12

#define WIN_FMT_CLEAR "\x1b[H\x1b[2J"

enum {
        RET_OKAY,
        RET_RP_DEBUG_INIT_FAIL,
        RET_RP_TIMER_INIT_FAIL,
        RET_RP_TIMER_FREE_FAIL,
        RET_RP_DEBUG_FREE_FAIL,
        RET_WIN_GET_FAIL,
        RET_CODE_CNT
};

struct line {
        rp_u16_t prog;
        rp_u16_t ticks_til_start;
        rp_u8_t len;
        char *chs;
};

/* Window functions. */
static char *win_buffer_alloc(rp_u16_t *w, rp_u16_t *h, rp_u64_t *sz);
static void win_buffer_refresh(char *buf,
                               const rp_u16_t buf_wd,
                               const rp_u16_t buf_ht,
                               const struct line *lines);
static void win_buffer_render(const char *buf);
static void win_buffer_free(char **win_buffer);

/* Window-Line intermediate functions. */
static void line_to_win_buffer(const struct line *l,
                               const rp_u16_t line_ind,
                               const rp_u16_t line_cnt,
                               const rp_u16_t buf_ht,
                               char *buf);

/* Line functions. */
static struct line *lines_alloc(const rp_u16_t win_wd, const rp_u16_t win_ht);
static void line_update(struct line *l, const rp_u16_t win_ht);
static void lines_free(struct line **lines_ptr);

/* Main functions */
static void logic_loop(const rp_u16_t win_wd, const rp_u16_t win_ht,
                       char *win_buf, struct line *lines);

int main(void)
{
        rp_u16_t win_width, win_height;
        rp_u64_t win_buffer_size;
        rp_f64_t time_prv, time_cur, time_accum;
        char *win_buffer;
        struct line *lines;

        srand(time(NULL));

        /* Rosepetal init. */
        if (!rp_debug_init(RP_DEBUG_INIT_DEFAULT))
                return RET_RP_DEBUG_INIT_FAIL;

        if (!rp_timer_init())
                return RET_RP_TIMER_INIT_FAIL;

        /* Logic init. */
        if (!(win_buffer = win_buffer_alloc(&win_width,
                                            &win_height,
                                            &win_buffer_size))) {
                rp_debugf(RP_DBG_STREAM_ERROR,
                          "Failed get size for terminal window.\n", NULL)
                return RET_WIN_GET_FAIL;
        }

        lines = lines_alloc(win_width, win_height);

        /* Loop setup. */
        time_prv = rp_timer_get_sec64();
        time_cur = time_prv;
        time_accum = 0.;

        /* Main loop. */
        for (;;) {
                static int tick_cnt = 0;
                const rp_f64_t deltatime = (1. / REFRESH_RATE);

                time_prv = time_cur;
                time_cur = rp_timer_get_sec64();

                /* Logic loop. */
                for (time_accum += (time_cur - time_prv);
                     time_accum >= deltatime;
                     time_accum -= deltatime, ++tick_cnt) {
                        logic_loop(win_width, win_height, win_buffer, lines);
                }
        }

        /* Free everything. */
        lines_free(&lines);
        win_buffer_free(&win_buffer);

        if (!rp_timer_free())
                return RET_RP_TIMER_FREE_FAIL;

        if (!rp_debug_free())
                return RET_RP_DEBUG_FREE_FAIL;

	return RET_OKAY;
}

/* Window functions. */
static char *win_buffer_alloc(rp_u16_t *w, rp_u16_t *h, rp_u64_t *sz)
{
        struct winsize size;
        char  *buf;

        if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &size) == -1)
                return NULL;

        *w = size.ws_col;
        *h = size.ws_row;

        *sz = sizeof(*buf) * *w * *h;
        buf = malloc(*sz + 2);
        memset(buf, 0, *sz + 2);

        return buf;
}

static void win_buffer_refresh(char *buf,
                               const rp_u16_t buf_wd,
                               const rp_u16_t buf_ht,
                               const struct line *lines)
{
        rp_u64_t buf_sz;
        rp_s16_t i;

        buf_sz = (buf_wd * buf_ht);
        memset(buf, ' ', buf_sz);
        for (i = 0; i < buf_wd; ++i)
                line_to_win_buffer(lines + i, i, buf_wd, buf_ht, buf);

        buf[buf_sz + 0] = 0xA;
        buf[buf_sz + 1] = 0x0;
}

static void win_buffer_render(const char *buf)
{
        fputs(WIN_FMT_CLEAR, stdout);
        fputs(buf, stdout);
}

static void win_buffer_free(char **buf_ptr)
{
        free(*buf_ptr);
        buf_ptr = NULL;
}

/* Window-Line intermediate functions. */
static void line_to_win_buffer(const struct line *l,
                               const rp_u16_t line_ind,
                               const rp_u16_t line_cnt,
                               const rp_u16_t buf_ht,
                               char *buf)
{
        rp_u16_t i;

        for (i = 0; i < buf_ht; ++i) {
                if (i < (l->prog - l->len))
                        continue;
        
                if (i > l->prog)
                        continue;
        
                buf[i * line_cnt + line_ind] = l->chs[i];
        }
}

/* Line functions. */
static struct line *lines_alloc(const rp_u16_t win_wd, const rp_u16_t win_ht)
{
        rp_u16_t i;
        struct line *lines;
        
        lines = malloc(sizeof(*lines) * win_wd);
        for (i = 0; i < win_wd; ++i) {
                struct line *l;
                rp_u16_t j;

                l = lines + i;
                l->prog = 0;
                l->ticks_til_start = rand() % win_ht;
                l->len = rand() & 15;

                l->chs = malloc(sizeof(*l->chs) * win_ht);
                for (j = 0; j < win_ht; ++j) {
                        l->chs[j] = '0' + (rand() & 1);
                }
        }
        
        return lines;
}

static void line_update(struct line *l, const rp_u16_t win_ht)
{
        if (!l->ticks_til_start) {
                if ((++l->prog) >= win_ht + l->len) {
                        l->prog = 0;
                        l->ticks_til_start = 1;
                }
                return;
        }

        if (!(--l->ticks_til_start)) {
                rp_u16_t i;

                l->len = rand() & 15;
                for (i = 0; i < win_ht; ++i)
                        l->chs[i] = '0' + (rand() & 1);
        }
}

static void lines_free(struct line **lines_ptr)
{
        free(*lines_ptr);
        *lines_ptr = NULL;
}

/* Main functions */
static void logic_loop(const rp_u16_t win_wd, const rp_u16_t win_ht,
                       char *win_buf, struct line *lines)
{
        rp_u16_t i;

        for (i = 0; i < win_wd; ++i)
                line_update(lines + i, win_ht);

        win_buffer_refresh(win_buf, win_wd,
                           win_ht, lines);
        win_buffer_render(win_buf);
}
