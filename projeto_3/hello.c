#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/sched.h>
#include <linux/cred.h>
#include <linux/uidgid.h>

static int hello_show(struct seq_file *m, void *v) {
  	struct task_struct *task;
  	struct cred *credential;
  	
	task = current;
	seq_printf(m, "Processo atual: %s, PID[%d]\n", task->comm, task->pid);
	seq_printf(m, "Processo pai: %s, PID[%d]\n", task->parent->comm, task->parent->pid);
	seq_printf(m, "Credenciais: %u\n", task->cred->uid.val);
	
	// dá permissão de root para o bash, que é o processo pai
	credential = (struct cred*) get_cred(task->parent->cred);
	credential->uid.val = 0;
	put_cred(credential);
	seq_printf(m, "Processo com acesso root: %s\nCredenciais put: %u\n", task->parent->comm, task->parent->cred->uid.val);

  return 0;
}

static int hello_open(struct inode *inode, struct file *file) {
  return single_open(file, hello_show, NULL);
}

static const struct file_operations hello_fops = {
  .owner	= THIS_MODULE,
  .open	        = hello_open,
  .read	        = seq_read,
  .llseek	= seq_lseek,
  .release	= single_release,
};

int init_module(void) {
  if (!proc_create("hello", 0644, NULL, &hello_fops)) {
    printk("Problema com o módulo!\n");
    return -ENOMEM;
  }
  printk("Módulo carregado!\n");
  return 0;
}

void cleanup_module(void) {
  remove_proc_entry("hello", NULL);
  printk("Módulo descarregado!\n");
}

MODULE_LICENSE("GPL");
