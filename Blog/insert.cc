//////////////////////////////////////////////////////////////////
// 通过这个程序使用MySQL API 实现数据库的插入功能
////////////////////////////////////////////////////////////////


#include <cstdio>
#include <cstdlib>
// 编译器默认从 /usr/include 目录中查找头文件
// mysql.h是在一个 mysql 的文件中
#include <mysql/mysql.h>


int main() {
  // 1. 创建一个数据库句柄
  MYSQL* connect_fd = mysql_init(NULL);
  // 2.和数据库建立连接(和TCP的区分开，这是在应用层建立连接)
  //  连接过程需要指定一些必要的信息
  //  a) 连接句柄
  //  b) 服务器的ip地址
  //  c) 用户名
  //  d) 密码
  //  e) 数据库名(blog_system)
  //  f) 服务器的端口号
  //  g) unix_sock NULL
  //  h) client_flog 0
  //  
  if(mysql_real_connect(connect_fd, "127.0.0.1", "root", "", "blog_system" , 3306, NULL , 0) == NULL){
    printf("连接失败! %s\n", mysql_error(connect_fd));
    return 1;
  }
  printf("连接成功!\n");
  // 3. 设计编码格式 utf8
  //  mysql server 部分 最初安装的时候已经设置成了 utf8
  //  也得在客户端这边也设置成 utf8
  mysql_set_character_set(connect_fd, "utf8");
  // 4. 拼装 SQL 语句
  char sql[1024 * 4] = {0};
  char title[] = "立一个flag";
  char content[] = "30w 的梦想";
  int tag_id = 1;
  char datetime[] = "2019/07/25 16:26";
  sprintf(sql,"insert into blog_table values(null, '%s','%s', %d,'%s')", title, content,  tag_id, datetime);
  printf("sql:%s\n", sql);
  // 5. 让 数据库 服务器执行 SQL 语句
  int ret = mysql_query(connect_fd, sql);
  if(ret < 0){
    printf("执行 sql 失败! %s\n", mysql_error(connect_fd));
    mysql_close(connect_fd);
    return 1;
  }
  // 6. 关闭句柄
  mysql_close(connect_fd);
  printf("执行成功!\n");
  return 0;
}
