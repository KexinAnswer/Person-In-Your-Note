#include <cstdio>
#include <cstdlib>
#include <mysql/mysql.h>

int main() {
  // 1. 创建一个句柄
  MYSQL* connect_fd = mysql_init(NULL);
  // 2. 建立连接
  // mysql_init 返回的指针 句柄
  // 用户名
  // 密码
  // 数据库名
  // 端口号
  // unix_socket
  // client_flag
  if(mysql_real_connect(connect_fd, "127.0.0.1", "root", "", "blog_system", 3306, NULL, 0) == NULL ){
    printf("连接失败! %s\n", mysql_error(connect_fd));
    return 1;
  }
  // 3. 设计编码格式
  mysql_set_character_set(connect_fd, "utf8");
  // 4. 拼接 SQL 语句
  char sql[1024 * 4] = {0};
  sprintf(sql, "select * from blog_table");
  // 5. 执行 SQL 语句
  int ret = mysql_query(connect_fd, sql);
  if(ret < 0){
    printf("执行sql失败! %s\n", mysql_error(connect_fd));
    return 1;
  }
  // 6. 遍历结果集合, MYSQL_RES select 得到的结果集合
  MYSQL_RES* result = mysql_store_result(connect_fd);
  if(result == NULL){
    printf("获取结果失败!%s\n", mysql_error(connect_fd));
    return 1;
  }
  //  a) 获取到结果集合中的 行数 和 列数
  int rows = mysql_num_rows(result);
  int fields = mysql_num_fields(result);
  //  b) 根据行数和列数 来遍历结果
  for(int i = 0; i < rows; ++i){
    //  一次获取到一行数据
    //  每次自动获取下一行数据
    MYSQL_ROW row = mysql_fetch_row(result);
    for(int j = 0; j < fields; ++j){
      printf("%s\t", row[j]);
    }
    printf("\n");
  }
  // 释放结果集合， 容易遗忘的事情
  mysql_close(connect_fd);
  printf("执行成功!\n");
  return 0;
}
