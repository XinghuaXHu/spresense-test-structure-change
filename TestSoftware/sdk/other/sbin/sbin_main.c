
#include <sdk/config.h>
#include <stdio.h>

#include <arch/board/board.h>

#include <asmp/asmp.h>
#include <asmp/mptask.h>

#define err(format, ...)        fprintf(stderr, format, ##__VA_ARGS__)

#define LED0  97
#define LED1  98
#define LED2  99
#define LED3 100

static void led_init(void)
{
  board_gpio_write(LED0, -1);
  board_gpio_config(LED0, 0, false, false, PIN_FLOAT);
  board_gpio_write(LED1, -1);
  board_gpio_config(LED1, 0, false, false, PIN_FLOAT);
  board_gpio_write(LED2, -1);
  board_gpio_config(LED2, 0, false, false, PIN_FLOAT);
  board_gpio_write(LED3, -1);
  board_gpio_config(LED3, 0, false, false, PIN_FLOAT);
}

static void dump_cpuset(cpu_set_t *set)
{
  int i;

  printf("Assigned CPUs:");
  for (i = 0; i < 8; i++)
    {
      if (CPU_ISSET(i, set))
        {
          printf(" %d", i);
        }
    }
  printf("\n");
}

static int run_secure_binary(const char *name, int ncpus)
{
  mptask_t mt;
  cpu_set_t set;
  int ret;

  led_init();

  usleep(500);

  ret = mptask_init_secure(&mt, name);
  if (ret != 0)
    {
      err("mptask_init_secure failed. %d\n", ret);
      return 1;
    }

  ret = mptask_assign_cpus(&mt, ncpus);
  if (ret != 0)
    {
      err("mptask_assign_cpus() failed. %d\n", ret);
      return 2;
    }

  ret = mptask_getcpuidset(&mt, &set);
  if (ret != 0)
    {
      err("mptask_getcpuidset() failed. %d\n", ret);
      return 3;
    }
  dump_cpuset(&set);

  ret = mptask_exec(&mt);
  if (ret != 0)
    {
      err("mptask_exec() failed. %d\n", ret);
      return 4;
    }

  printf("%s is running a while.. ", name);
  fflush(stdout);
  sleep(3);

  ret = mptask_destroy(&mt, true, NULL);
  if (ret != 0)
    {
      err("mptask_destroy() failed. %d\n", ret);
      return 5;
    }
  printf("done\n");

  return 0;
}

#ifdef CONFIG_BUILD_KERNEL
int main(int argc, FAR char *argv[])
#else
int sbin_main(int argc, char *argv[])
#endif
{
  char dspname[32] = { "test1" };
  int ret;
  int i;

  printf("\n"
         "Start test for unified binary\n"
         "\n");

  for (i = 1; i < 5; i++)
    {
      sprintf(dspname, "test%d", i);
      ret = run_secure_binary(dspname, i);
      if (ret)
        {
          break;
        }
    }

  printf("\n"
         "Start test for cloning\n"
         "\n");

  sprintf(dspname, "test1");
  for (i = 1; i < 5; i++)
    {
      ret = run_secure_binary(dspname, i);
      if (ret)
        {
          break;
        }
    }

  return 0;
}
