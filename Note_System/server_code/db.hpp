#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <memory>
#include <jsoncpp/json/json.h>
#include <mysql/mysql.h>

/////////////////////////////////////////////////////////////////
//创建一些相关类来封装数据库操作
///////////////////////////////////////////////////////
// 博客管理 API 设计
// 新增博客
// 请求：
// POST / blog
// {
//   "title" : "我的第一篇博客",
//   "content" : "我的博客 markdown 格式内容",
//   "tag_id" : 1,
//   "create_time" : "2019/07/26 12:00"
// }
//
// 响应：
// HTTP/1.1 200 OK
// {
//   "ok" : true,
// }

// 获取所有博客标题
// 请求（使用 tag_id 参数筛选指定标签下的博客
// GET / blog?tag_id = 1
//
// 响应
// HTTP/1.1 200 OK
// [
//    {
//      "blog_id" : 1,
//      "blog_url" : "/blog/1",
//      "title" : "我的第一篇博客",
//      "tag_id" : 1,
//      "create_time" : "2019/07/26 12:00",
//    },
//    {
//      "blog_id" : 2,
//      "blog_url" : "blog/2",
//      "title" : "C 语言的类型",
//      "tag_id" : 1,
//      "create_time" : "2019/07/26 13:00",
//    }
// ]
//
// 删除博客
// 请求
// DELETE/blog/:blog_id
//
// 响应
// {
//   "ok" : true,
// }
// 
// 修改博客
// 请求:
// PUT /blog/:blog_id
// {
//   "title" : "我的第一篇博客",
//   "content" : "博客的 markdown 格式内容",
//   "tag_id" : 1,
// }
//
// 响应
// HTTP/1.1 200 OK
// {
//   "ok" : true,
// }
// 
// 获取博客详细内容
// GET /blog/:blog_id 
//
// 响应
// HTTP/1.1/ 200 OK
// {
//   "blog_id" : 1,
//   "title" : "我的第一篇博客",
//   "content" : "博客的 markdown 格式内容",
//   "tag_id" : 1,
// }
//
//
// 标签管理 API 设计
// 新增标签
// 请求
// POST /tag
// {
//   "tag_name" : "C 语言",
// }
//
// 响应
// HTTP/1.1 200 OK
// {
//   "ok" : true,
// }
//
// 删除标签
// 请求：
// DELETE /tag/:tag_id
//
// 响应
// HTTP/1.1 200 OK
// {
//   "ok" : true,
// }
//
// 获取所有标签
// GET /tag
//
// 响应
// [
//   {
//     "tag_id" : 1,
//     "tag_name" : "C 语言"
//   },
//   {
//     "tag_id" : 2,
//     "tag_name" : "数据结构"
//   }
//
// ]

static MYSQL* MySQLInit(){
  // 初始化一个 MYSQL 句柄并建立连接
  // 1.创建句柄
  MYSQL* connect_fd = mysql_init(NULL);
  // 2. 和数据库建立连接
  // mysql_real_connect 
  // 句柄
  // 主机ip地址
  // 数据库用户
  // 数据库密码
  // 数据库名
  // 端口号
  // unix_sock
  // client_flag
  if(mysql_real_connect(connect_fd, "127.0.0.1", "root", "","blog_system", 3306, NULL, 0) == NULL){

    printf("连接失败！ %s\n", mysql_error(connect_fd));
    return NULL;
  }
  // 3.设计编码格式 为 utf8
  mysql_set_character_set(connect_fd, "utf8");
  return connect_fd;
}

static void MySQLRelease(MYSQL* mysql){
  // 释放句柄
  mysql_close(mysql);
}

class BlogTable{
  public:
    // 通过这个构造函数获取到一个 数据库 的操作句柄
    BlogTable(MYSQL* mysql):mysql_(mysql){  }

    // 一下操作相关参数都统一使用 JSON 的方式
    // Json::Value jsoncpp 中最核心的类
    // Json::Value 就表示一个具体的 json 对象
    // 最大的好处就是方便扩展
    // 实现插入博客
    bool Insert(const Json::Value& blog){
      // 核心就是拼装 SQL 语句
      // 如果拿一个完整的课件做实验，会发现：
      // 由于博客内容中可能会包含一些特殊字符(\n, '', ""等),会导致拼装处的 sql 语句有问题
      // 应该使用 mysql_real_escape_string 对 content 字段来进行转义
      // 转义只是为了让 SQL 语句拼接正确，实际上插入成功后数据库的内容已经自动转义回来了
      const std::string& content = blog["content"].asString();
      // 文档上要求战役的缓冲区长度必须是执勤的 2 倍 + 1
      // 使用 unique_ptr 管理内存
      std::unique_ptr<char> content_escape(new char[content.size() * 2 + 1]);
      mysql_real_escape_string(mysql_, content_escape.get(), content.c_str(), content.size());
      // 插入的博客时间可能较长， 需要搞个大一点的缓冲区(根据用户请求的长度自适应)
      std::unique_ptr<char> sql(new char[content.size() * 2 + 4096]);
      sprintf(sql.get(), "insert into blog_table values(null, '%s', '%s' , %d, '%s')", 
          blog["title"].asCString(), 
          content_escape.get(), 
          blog["tag_id"].asInt(),
          blog["create_time"].asCString());
      int ret = mysql_query(mysql_,sql.get());
      printf("sql:%s\n", sql.get());
      if(ret != 0){
        printf("执行 sql 失败! sql = %s, %s\n", sql.get(), mysql_error(mysql_));
        return false;
      }
      printf("执行插入博客成功!");
      return true;
    }

    // blogs 作为一个输出型参数，
    // 可以通过 tag_id 查找一个
    // 也可以全部查找
    bool SelectAll(Json::Value* blogs, const std::string& tag_id = ""){
      // 查找不需要太长的 sql 固定长度就可以了
      char sql[1024 * 4] = {0};
      // 可以根据 tag_id 来筛选结果
      if(tag_id.empty()){
        // 此时不需要 tag 来筛选结果
        sprintf(sql,"select blog_id, title, tag_id , create_time from blog_table");
      } else{
        // 需要 tag 来筛选
        sprintf(sql,"select blog_id, title, tag_id, create_time from blog_table, where tag_id  = %d",std::stoi(tag_id));
      }
      int ret = mysql_query(mysql_,sql);
      if( ret != 0){
        printf("执行 sql 失败!%s \n", mysql_error(mysql_));
        return false;
      }
      MYSQL_RES* result = mysql_store_result(mysql_);
      if(result == NULL){
        printf("获取结果失败! %s\n", mysql_error(mysql_));
        return false;
      }
      // 遍历结果集合 把结果写到 blogs 参数中 ， 返回调用者
      int rows = mysql_num_rows(result);
      for(int i = 0 ; i < rows; ++i){
        MYSQL_ROW row = mysql_fetch_row(result);
        Json::Value blog;
        // atoi C 风格字符串 转成 整数
        // row[] 中的下标和上面的select 语句中写的列的顺序是相关的
        blog["blog_id"] = atoi(row[0]);
        blog["title"] = row[1];
        blog["tag_id"] = atoi(row[2]);
        blog["create_time"] = row[3];
        // 遍历结果依次加入到 dishes 中
        printf("%d\n %s\n %d\n %s\n", atoi(row[0]), row[1], atoi(row[2]) , row[3]);
        printf("blogs->append(blog)\n");
        blogs->append(blog);
      }
      // mysql 查询的结果集合需要记得释放
      mysql_free_result(result);
      printf("执行查找博客成功!, 共查找到 %d 条博客\n" , rows);
      return true;
    }

    // blog 同样是输出型参数，根据当前的 blog_id 在数据中找到具体的
    // 博客内容通过 blog 参数返回给调用者
    // TODO
    // 看录屏
    bool SelectOne(int32_t blog_id, Json::Value* blog){
      char sql[1024 * 4] = {0}; 
      sprintf(sql,"select blog_id, title,content, tag_id, create_time from blog_table where blog_id = %d", blog_id);
      printf("sql:%s\n",sql);
      int ret = mysql_query(mysql_,sql); 
      if(ret != 0){
        printf("执行 sql 失败!%s \n", mysql_error(mysql_));
        return false;
      }
      MYSQL_RES* result = mysql_store_result(mysql_);
      if(result == NULL){
        printf("获取结果失败! %s\n", mysql_error(mysql_));
        return false;
      }
      int rows = mysql_num_rows(result);
      if(rows != 1){
        printf("查找结果不止 1 条。 rows = %d", rows);
        return false;
      }
      MYSQL_ROW row = mysql_fetch_row(result);
      (*blog)["blog_id"] = atoi(row[0]);
      (*blog)["title"] = row[1];
      (*blog)["content"] = row[2];
      (*blog)["tag_id"] = atoi(row[3]);
      (*blog)["create_time"] = row[4];
      printf("查找成功！\n");
      return true;
    }

    bool Update(const Json::Value& blog){
      // 如果一个完整的课件做实验， 会发现:
      // 由于博客内容中柯南包含一些特殊字符(\n, '', ""等), 会导致拼接出来的 sql 语句有问题
      // 应该使用 mysql_query_espace_string 对 content 字段进行转义
      // 转义只是为了让 SQL 语句拼接正确，实际上插入成功后的数据库的内容已经自动转义回来了
      const std::string& content = blog["content"].asString();
      // 文档上要求转义的缓冲区长度必须是之前的 2 倍 + 1
      // 使用 unique_ptr 管理内存
      std::unique_ptr<char> content_espace(new char[content.size() * 2 + 1]);
      mysql_real_escape_string(mysql_, content_espace.get(), content.c_str(), content.size());
      // 插入博客的内容可能较长,需要搞一个大点的缓冲区(根据用户请求的长度自适应)
      std::unique_ptr<char> sql(new char[content.size() * 2 + 4096]);
      sprintf(sql.get(),"update blog_table SET title='%s', content='%s',  tag_id=%d where blog_id=%d",
          blog["title"].asCString(),
          content_espace.get(),
          blog["tag_id"].asInt(),
          blog["blog_id"].asInt());

      // DEBUG 用于调试
       printf("[SQL]%s\n", sql.get());

      int ret = mysql_query(mysql_,sql.get());
      if(ret != 0){
        printf("执行sql失败! 更新失败 %s\n", mysql_error(mysql_));
        return false;
      }
      printf("更新博客成功!\n");
      return true;
    }

    bool Delete(int32_t blog_id){
      char sql[1024]  = {0};
      sprintf(sql,"delete from blog_table where blog_id=%d", blog_id);
      int ret = mysql_query(mysql_, sql);
      if(ret != 0){
        printf("执行 sql 失败! sql=%s, %s\n", sql,mysql_error(mysql_));
        return false;
      }
      return true;
    }

  private:
    MYSQL* mysql_;
};

class TagTable{
  public:
    TagTable(MYSQL* mysql):mysql_(mysql){  }

    bool SelectAll(Json::Value* tags){
      char sql[1024*4] = {0};
      sprintf(sql,"select * from tag_table");
      int ret = mysql_query(mysql_,sql);
      if(ret != 0){
        printf("获取结果失败! 1 %s\n", mysql_error(mysql_));
        return false;
      }
      MYSQL_RES* result = mysql_store_result(mysql_);
      if(result == NULL){
        printf("获取结果失败! 2 %s\n", mysql_error(mysql_));
        return false;
      }
      int rows = mysql_num_rows(result);
      for(int i = 0; i < rows; ++i){
        MYSQL_ROW row = mysql_fetch_row(result);
        Json::Value tag;
        tag["tag_id"] = atoi(row[0]);
        tag["tag_name"] = row[1];
        tags->append(tag);
      }
      return true;
      printf("查找成功！ 共找到 %d 个", rows);
    }

    bool Insert(const Json::Value& tag){
      char sql[1024 * 4] = {0};
      // 此处 dish_ids 需要先转换成字符串(本来是一个对象，
      // 形如 [1,2,3] 如果不穿， 是无法 asCString)
      sprintf(sql,"insert into tag_table values(null, '%s')", tag["tag_name"].asCString());
      int ret = mysql_query(mysql_,sql);
      if(ret != 0){
        printf("执行 sql 失败! sql=%s, %s\n", sql, mysql_error(mysql_));
        return false;
      }
      return true;
    }

    bool Delete(int32_t tag_id){
      char sql[1024*4] = {0};
      sprintf(sql,"delete from tag_table where tag_id = %d", tag_id);
      int ret = mysql_query(mysql_,sql);
      if(ret != 0){
        printf("执行 sql 失败! sql=%s ,%s", sql,mysql_error(mysql_));
        return false;
      }
      return true;
    }

  private:
    MYSQL* mysql_;
};
