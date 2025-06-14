#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <stdbool.h>

#define MAX_MSG_LEN 128      // 最大日志消息长度
#define PAGE_SIZE 10         // 每页显示的日志数量

// 日志级别枚举
typedef enum {
    DEBUG,
    INFO,
    WARNING,
    ERROR,
    CRITICAL
} LogLevel;

// 日志单链表结构体
typedef struct LogEntry {
    time_t timestamp;
    LogLevel level;
    char message[MAX_MSG_LEN];
    struct LogEntry* next;
} LogEntry;

// 日志系统结构体
typedef struct {
    LogEntry* head;         // 链表头指针
    LogEntry* tail;         // 链表尾指针
    int count;              // 日志总数
    int current_page;       // 当前页码
    int total_pages;        // 总页数
} LogSystem;

// 初始化日志系统
void init_log_system(LogSystem* sys) {
    sys->head = NULL;
    sys->tail = NULL;
    sys->count = 0;
    sys->current_page = 1;
    sys->total_pages = 1;
}

// 将日志级别转换为字符串
const char* level_to_string(LogLevel level) {
    switch (level) {
        case DEBUG:    return "DEBUG";
        case INFO:     return "INFO";
        case WARNING:  return "WARNING";
        case ERROR:    return "ERROR";
        case CRITICAL: return "CRITICAL";
        default:       return "UNKNOWN";
    }
}

// 创建新的日志节点
LogEntry* create_log_entry(LogLevel level, const char* message) {
    LogEntry* new_entry = (LogEntry*)malloc(sizeof(LogEntry));
    if (!new_entry) {
        perror("内存分配失败");
        return NULL;
    }
    
    new_entry->timestamp = time(NULL);
    new_entry->level = level;
    strncpy(new_entry->message, message, MAX_MSG_LEN - 1);
    new_entry->message[MAX_MSG_LEN - 1] = '\0';
    new_entry->next = NULL;
    
    return new_entry;
}

// 添加日志到链表尾部
void add_log(LogSystem* sys, LogLevel level, const char* message) {
    LogEntry* new_entry = create_log_entry(level, message);
    if (!new_entry) return;
    
    if (sys->head == NULL) {
        // 链表为空
        sys->head = new_entry;
        sys->tail = new_entry;
    } else {
        // 添加到链表尾部
        sys->tail->next = new_entry;
        sys->tail = new_entry;
    }
    
    sys->count++;
    // 更新总页数
    sys->total_pages = (sys->count + PAGE_SIZE - 1) / PAGE_SIZE;
    
    printf("日志添加成功！\n");
}

// 删除指定索引的日志
bool delete_log(LogSystem* sys, int index) {
    if (index < 0 || index >= sys->count) {
        printf("无效的日志索引！\n");
        return false;
    }
    
    LogEntry* current = sys->head;
    LogEntry* prev = NULL;
    
    // 遍历到要删除的节点
    for (int i = 0; i < index; i++) {
        prev = current;
        current = current->next;
    }
    
    // 删除节点
    if (prev == NULL) {
        // 删除头节点
        sys->head = current->next;
        if (sys->tail == current) {
            sys->tail = NULL; // 如果只有一个节点
        }
    } else {
        prev->next = current->next;
        if (sys->tail == current) {
            sys->tail = prev; // 如果删除的是尾节点
        }
    }
    
    free(current);
    sys->count--;
    
    // 更新总页数
    sys->total_pages = (sys->count + PAGE_SIZE - 1) / PAGE_SIZE;
    
    // 调整当前页码，防止超出范围
    if (sys->current_page > sys->total_pages && sys->total_pages > 0) {
        sys->current_page = sys->total_pages;
    }
    
    return true;
}

// 更新指定索引的日志
//  参数：sys - 日志系统
//        index - 要更新的日志索引
//        new_message - 新的日志消息
//  返回 true 表示成功，false 表示失败
bool update_log(LogSystem* sys, int index, const char* new_message) {
    if (index < 0 || index >= sys->count) {
        printf("无效的日志索引！\n");
        return false;
    }
    
    LogEntry* current = sys->head;
    
    // 遍历到要更新的节点
    for (int i = 0; i < index; i++) {
        current = current->next;
    }
    
    // 更新日志
    if (strlen(new_message) > MAX_MSG_LEN - 1) {
        printf("新日志消息过长，将被截断！\n");
    }
    
    strncpy(current->message, new_message, MAX_MSG_LEN - 1); 
    current->message[MAX_MSG_LEN - 1] = '\0';
    
    // 更新日志时间戳
    current->timestamp = time(NULL);
    
    return true;
}

// 显示单条日志
void display_log(LogEntry* log) {
    struct tm* timeinfo = localtime(&log->timestamp);
    char time_str[20];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", timeinfo);
    
    printf("[%s] %-9s %s\n", time_str, level_to_string(log->level), log->message);
}

// 显示日志分页
void display_logs_page(LogSystem* sys) {
    if (sys->count == 0) {
        printf("\n日志系统为空！\n");
        return;
    }
    
    // 计算当前页的开始和结束索引
    int start_index = (sys->current_page - 1) * PAGE_SIZE;
    int end_index = start_index + PAGE_SIZE;
    
    if (end_index > sys->count) {
        end_index = sys->count;
    }
    
    printf("\n=== 日志列表 (第 %d/%d 页, 共 %d 条日志) ===\n", 
           sys->current_page, sys->total_pages, sys->count);
    
    // 遍历链表到起始位置
    LogEntry* current = sys->head;
    for (int i = 0; i < start_index; i++) {
        current = current->next;
    }
    
    // 显示当前页的日志
    for (int i = start_index; i < end_index; i++) {
        printf("%4d. ", i + 1);
        display_log(current);
        current = current->next;
    }
    
    // 显示分页导航
    printf("\n导航: ");
    if (sys->current_page > 1) {
        printf("[P] 上一页 ");
    }
    if (sys->current_page < sys->total_pages) {
        printf("[N] 下一页 ");
    }
    printf("[F] 首页 [L] 尾页 [Q] 返回主菜单\n");
}

// 搜索日志（按关键词）
void search_logs(LogSystem* sys, const char* keyword) {
    if (sys->count == 0) {
        printf("\n日志系统为空！\n");
        return;
    }
    
    printf("\n=== 搜索结果 (关键词: \"%s\") ===\n", keyword);
    int found = 0;
    int index = 0;
    
    LogEntry* current = sys->head;
    while (current != NULL) {
        // 在日志消息中搜索关键词（不区分大小写）
        //if (strcasestr(current->message, keyword) != NULL) {
        if (strstr(current->message, keyword) != NULL) {
            printf("%4d. ", index + 1);
            display_log(current);
            found++;
        }
        current = current->next;
        index++;
    }
    
    if (found == 0) {
        printf("未找到包含关键词 \"%s\" 的日志\n", keyword);
    } else {
        printf("找到 %d 条相关日志\n", found);
    }
}

// 释放所有日志内存
void free_all_logs(LogSystem* sys) {
    LogEntry* current = sys->head;
    while (current != NULL) {
        LogEntry* next = current->next;
        free(current);
        current = next;
    }
    
    sys->head = NULL;
    sys->tail = NULL;
    sys->count = 0;
    sys->current_page = 1;
    sys->total_pages = 1;
}

// 显示菜单
void display_menu() {
    printf("\n=== C语言链表日志系统 ===\n");
    printf("1. 添加日志\n");
    printf("2. 查看日志\n");
    printf("3. 删除日志\n");
    printf("4. 修改日志\n");
    printf("5. 搜索日志\n");
    printf("6. 清空所有日志\n");
    printf("7. 退出系统\n");
    printf("请选择操作: ");
}

// 添加日志菜单
void add_log_menu(LogSystem* sys) {
    printf("\n--- 添加日志 ---\n");
    
    int level_choice;
    char message[MAX_MSG_LEN];
    
    printf("选择日志级别:\n");
    printf("1. 调试(DEBUG)\n");
    printf("2. 信息(INFO)\n");
    printf("3. 警告(WARNING)\n");
    printf("4. 错误(ERROR)\n");
    printf("5. 严重(CRITICAL)\n");
    printf("请选择(1-5): ");
    scanf("%d", &level_choice);
    getchar(); // 清除输入缓冲区
    
    if (level_choice < 1 || level_choice > 5) {
        printf("无效的选择！\n");
        return;
    }
    
    LogLevel level;
    switch (level_choice) {
        case 1: level = DEBUG; break;
        case 2: level = INFO; break;
        case 3: level = WARNING; break;
        case 4: level = ERROR; break;
        case 5: level = CRITICAL; break;
    }
    
    printf("输入日志内容 (最多 %d 字符): ", MAX_MSG_LEN - 1);
    fgets(message, MAX_MSG_LEN, stdin);
    message[strcspn(message, "\n")] = '\0'; // 移除换行符
    
    add_log(sys, level, message);
}

// 查看日志菜单
void view_logs_menu(LogSystem* sys) {
    sys->current_page = 1; // 重置为第一页
    display_logs_page(sys);
    
    char choice;
    while (1) {
        printf("\n请输入导航命令: ");
        scanf(" %c", &choice);
        choice = toupper(choice);
        
        switch (choice) {
            case 'P': // 上一页
                if (sys->current_page > 1) {
                    sys->current_page--;
                    display_logs_page(sys);
                } else {
                    printf("已经是第一页！\n");
                }
                break;
                
            case 'N': // 下一页
                if (sys->current_page < sys->total_pages) {
                    sys->current_page++;
                    display_logs_page(sys);
                } else {
                    printf("已经是最后一页！\n");
                }
                break;
                
            case 'F': // 首页
                sys->current_page = 1;
                display_logs_page(sys);
                break;
                
            case 'L': // 尾页
                sys->current_page = sys->total_pages;
                display_logs_page(sys);
                break;
                
            case 'Q': // 返回主菜单
                return;
                
            default:
                printf("无效的命令！请使用 P/N/F/L/Q\n");
        }
    }
}

// 删除日志菜单
void delete_log_menu(LogSystem* sys) {
    printf("\n--- 删除日志 ---\n");
    if (sys->count == 0) {
        printf("日志系统为空！\n");
        return;
    }
    
    display_logs_page(sys);
    printf("\n输入要删除的日志编号 (0 取消): ");
    
    int index;
    scanf("%d", &index);
    
    if (index == 0) {
        return;
    }
    
    if (index < 1 || index > sys->count) {
        printf("无效的日志编号！\n");
        return;
    }
    
    if (delete_log(sys, index - 1)) {
        printf("日志 %d 已删除！\n", index);
    }
}

// 修改日志菜单
void update_log_menu(LogSystem* sys) {
    printf("\n--- 修改日志 ---\n");
    if (sys->count == 0) {
        printf("日志系统为空！\n");
        return;
    }
    
    display_logs_page(sys);
    printf("\n输入要修改的日志编号 (0 取消): ");
    
    int index;
    scanf("%d", &index);
    getchar(); // 清除输入缓冲区
    
    if (index == 0) {
        return;
    }
    
    if (index < 1 || index > sys->count) {
        printf("无效的日志编号！\n");
        return;
    }
    
    char new_message[MAX_MSG_LEN];
    printf("输入新的日志内容 (最多 %d 字符): ", MAX_MSG_LEN - 1);
    fgets(new_message, MAX_MSG_LEN, stdin);
    new_message[strcspn(new_message, "\n")] = '\0'; // 移除换行符
    
    if (update_log(sys, index - 1, new_message)) {
        printf("日志 %d 已更新！\n", index);
    }
}

// 搜索日志菜单
void search_logs_menu(LogSystem* sys) {
    printf("\n--- 搜索日志 ---\n");
    if (sys->count == 0) {
        printf("日志系统为空！\n");
        return;
    }
    
    char keyword[MAX_MSG_LEN];
    printf("输入搜索关键词: ");
    fgets(keyword, MAX_MSG_LEN, stdin);
    keyword[strcspn(keyword, "\n")] = '\0'; // 移除换行符
    
    if (strlen(keyword) == 0) {
        printf("关键词不能为空！\n");
        return;
    }
    
    search_logs(sys, keyword);
}

// 清空所有日志
void clear_all_logs(LogSystem* sys) {
    printf("\n确定要清空所有日志吗？(y/n): ");
    
    char choice;
    scanf(" %c", &choice);
    
    if (toupper(choice) == 'Y') {
        free_all_logs(sys);
        printf("所有日志已清空！\n");
    } else {
        printf("操作已取消\n");
    }
}

// 生成示例日志
void generate_sample_logs(LogSystem* sys) {
    add_log(sys, INFO, "系统启动完成");
    add_log(sys, DEBUG, "初始化配置参数");
    add_log(sys, WARNING, "磁盘空间不足，仅剩 10%");
    add_log(sys, ERROR, "无法连接到数据库服务器");
    add_log(sys, INFO, "用户 'admin' 登录成功");
    add_log(sys, DEBUG, "加载用户权限信息");
    add_log(sys, CRITICAL, "关键服务异常终止");
    add_log(sys, INFO, "创建新用户 'testuser'");
    add_log(sys, WARNING, "检测到多次登录失败尝试");
    add_log(sys, ERROR, "文件写入失败: /var/log/app.log");
    add_log(sys, INFO, "系统备份完成");
    add_log(sys, DEBUG, "释放内存资源");
}

int main() {
    LogSystem sys;
    init_log_system(&sys);
    
    // 生成示例日志
    generate_sample_logs(&sys);
    
    int choice;
    while (1) {
        display_menu();
        if (scanf("%d", &choice) != 1) {
            printf("无效输入！\n");
            while (getchar() != '\n'); // 清除输入缓冲区
            continue;
        }
        
        switch (choice) {
            case 1:
                add_log_menu(&sys);
                break;
            case 2:
                view_logs_menu(&sys);
                break;
            case 3:
                delete_log_menu(&sys);
                break;
            case 4:
                update_log_menu(&sys);
                break;
            case 5:
                search_logs_menu(&sys);
                break;
            case 6:
                clear_all_logs(&sys);
                break;
            case 7:
                printf("感谢使用日志系统，再见！\n");
                free_all_logs(&sys); // 释放所有内存
                return 0;
            default:
                printf("无效选择，请重新输入！\n");
        }
    }
    
    return 0;
}