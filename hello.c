#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/timer.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/ktime.h>

#define DEVICE_NAME "reaction_time"
#define INITIAL_BUFFER_SIZE 10

static struct timer_list my_timer;
static int frequency = 100; // Частота в Гц
static ktime_t event_time;
static ktime_t *response_time;
static int buffer_size = INITIAL_BUFFER_SIZE;
static int index = 0;

static int device_open(struct inode *inode, struct file *file) {
    index = 0; // Сброс индекса при открытии устройства
    return 0;
}

static ssize_t device_read(struct file *file, char __user *buffer, size_t len, loff_t *offset) {
    if (index == 0) {
        return 0; // Нет данных для чтения
    }

    // Ограничиваем количество байтов для чтения
    size_t count = min(len, sizeof(ktime_t) * index);

    // Копируем данные времени реакции в пользовательское пространство
    if (copy_to_user(buffer, response_time, count)) {
        return -EFAULT;
    }

    // Возвращаем количество байтов
    return count;
}

static ssize_t device_write(struct file *file, const char __user *buffer, size_t len, loff_t *offset) {
    if (index >= buffer_size) {
        // Увеличиваем размер буфера
        buffer_size *= 2; // Увеличиваем вдвое
        response_time = krealloc(response_time, sizeof(ktime_t) * buffer_size, GFP_KERNEL);
        if (!response_time) {
            return -ENOMEM; // Ошибка при выделении памяти
        }
    }

    ktime_t current_time = ktime_get();
    ktime_t reaction_time = ktime_sub(current_time, event_time); // Рассчитываем время реакции
    response_time[index++] = reaction_time; // Записываем время реакции

    // Выводим время реакции в лог ядра
    printk(KERN_INFO "Reaction Time: %lld ns\n", ktime_to_ns(reaction_time));

    return len; // Возвращаем количество записанных байтов
}

void timer_callback(struct timer_list *timer) {
    event_time = ktime_get(); // Сохраняем время события
    
    // Перезапускаем таймер
    mod_timer(&my_timer, jiffies + msecs_to_jiffies(1000 / frequency));
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = device_open,
    .read = device_read,
    .write = device_write,
};

static int __init reaction_time_init(void) {
    int result;

    // Регистрируем символьное устройство
    result = register_chrdev(0, DEVICE_NAME, &fops);
    if (result < 0) {
        printk(KERN_ALERT "Failed to register character device\n");
        return result;
    }

    // Выделяем память для хранения времени реакции
    response_time = kmalloc(sizeof(ktime_t) * buffer_size, GFP_KERNEL);
    if (!response_time) {
        unregister_chrdev(result, DEVICE_NAME);
        return -ENOMEM; // Ошибка при выделении памяти
    }

    // Настраиваем таймер
    timer_setup(&my_timer, timer_callback, 0);
    mod_timer(&my_timer, jiffies + msecs_to_jiffies(1000 / frequency));

    printk(KERN_INFO "Reaction Time Driver Initialized\n");
    return 0;
}

static void __exit reaction_time_exit(void) {
    del_timer(&my_timer);
    kfree(response_time); // Освобождаем память
    unregister_chrdev(0, DEVICE_NAME);
    printk(KERN_INFO "Reaction Time Driver Exited\n");
}

module_init(reaction_time_init);
module_exit(reaction_time_exit);

MODULE_LICENSE("GPL");
