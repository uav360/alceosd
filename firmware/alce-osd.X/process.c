/*
    AlceOSD - Graphical OSD
    Copyright (C) 2015  Luis Alves

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "alce-osd.h"


#define MAX_PROCESSES   10

struct process {
    void (*process)(void);
    unsigned long time, calls;
    unsigned char priority;
    unsigned char weight;
    const char *name;
};

static struct process process_list[MAX_PROCESSES] = { { .process = NULL} };
static unsigned char nr_processes = 0;


void process_add(void *f, const char *name, unsigned char priority)
{
    if (nr_processes < MAX_PROCESSES) {
        process_list[nr_processes].name = name;
        process_list[nr_processes].time = 0;
        process_list[nr_processes].calls = 0;
        process_list[nr_processes].priority = priority;
        process_list[nr_processes].weight = 100;
        process_list[nr_processes++].process = f;
        process_list[nr_processes].process = NULL;
    }
}

static struct process *get_next_process(void)
{
    struct process *p = process_list, *n = process_list;
    int i;
    unsigned char j;
    
    //shell_printf("\nweights: ");
    while (p->process != NULL) {
        //shell_printf("%s=%u ", p->name, p->weight);
        if (p->weight > n->weight)
            n = p;
        p++;
    }
    //shell_printf("RUN: %s", n->name);
    
    j = 100 - n->weight;
    i = (-4 * ((int)n->priority))/50 + 9;
    i = ((int) n->weight) - i;
    n->weight = (unsigned char) i;
    p = process_list;
    while (p->process != NULL) {
        (p++)->weight += j;
    }
    
    return n;
}


void process_run(void)
{
    struct process *p;
    unsigned int t;
    
    for (;;) {

        p = get_next_process();

        t = get_micros();
        p->process();
        t = get_micros() - t;
        p->calls++;
        p->time += t;

        //p++;
        //if (p->process == NULL) {
        //    p = process_list;
        //    ClrWdt();
        //}
    }
}

static void shell_cmd_stats(char *args, void *data)
{
    unsigned char i;
    float t;
    unsigned long total = 0, millis = get_millis(), total_calls = 0;
    
    for (i = 0; i < nr_processes; i++)
        total_calls += process_list[i].calls;
    
    shell_printf("\nProcess list\n");
    for (i = 0; i < nr_processes; i++) {
        t = ((float) process_list[i].time / 1000) / (float) millis * 100.0;
        total += process_list[i].time / 1000;
        shell_printf("%2d %-10s time(ms)=%6lu time(%%)=%.2f calls=%lu(%.2f%%) \n", i,
                process_list[i].name, process_list[i].time / 1000, t,
                process_list[i].calls,
                (float) process_list[i].calls/total_calls * 100.0);
    }
    total = millis - total;
    t = ((float) total) / (float) millis * 100.0;
    shell_printf("%2d %-10s time(ms)=%6lu time(%%)=%.2f\n", i,
            "IRQS", total, t);
}

static void shell_cmd_stack(char *args, void *data)
{
    unsigned char i;
    unsigned int *stack_pos;
    unsigned int *p, *t;
    unsigned char *c;
    
    asm volatile("mov.w #__SP_init,%0" : "=r"(p));
    asm volatile("mov.w #__SPLIM_init,%0" : "=r"(t));
    asm volatile("mov.w W15,%0" : "=r"(stack_pos));
    
    c = (unsigned char*) p;
    
    shell_printf("\nStack dump :: [0x%04p--->0x%04p   0x%04p]\n\n", p, stack_pos, t);
    i = 0;
    shell_printf("0x%4p | ", p);
    while (p < stack_pos) {
        shell_printf("%04x ", *p++);

        if (i++ == 7) {
            for (i = 0; i < 16; i++) {
                if ((*c) > 20)
                    shell_printf("%c", *c);
                c++;
            }
            shell_printf("\n0x%4p : ", p);
            i = 0;
        }
    }
}

static const struct shell_cmdmap_s process_cmdmap[] = {
    {"stats", shell_cmd_stats, "stats", SHELL_CMD_SIMPLE},
    {"stack", shell_cmd_stack, "dump stack", SHELL_CMD_SIMPLE},
    {"", NULL, ""},
};

void shell_cmd_process(char *args, void *data)
{
    shell_exec(args, process_cmdmap, data);
}

