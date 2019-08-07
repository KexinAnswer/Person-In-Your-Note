#include "db.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <jsoncpp/json/json.h>

void TestBlogTable(){
  bool ret = false;
  // 更友好的显示 Json
  Json::StyledWriter writer;
  MYSQL* mysql = MySQLInit();

  Json::Value blog;
  blog["title"] = "初识 C 语言";
  std::string content;
  FileUtil::ReadFile("./test_data/1.md", &content);
  blog["content"] = content;
  blog["tag_id"] = 1;
  blog["create_time"] = "2019/7/27 10:35";

  std::cout << "=================================================测试插入==========================================" << std::endl;
  BlogTable blog_table(mysql);
  ret = blog_table.Insert(blog);
  std::cout << "Insert" << ret << std::endl;

  std::cout << "=================================================测试查找==========================================" << std::endl;
  Json::Value blogs;
  ret = blog_table.SelectAll(&blog);
  std::cout << "selectAll:" << ret << std::endl << writer.write(blogs) << std::endl;
  
  std::cout << "=================================================测试更新==========================================" << std::endl;
  blog["blog_id"] = 1;
  blog["title"]  = "测试更新博客";
  blog["content"] = content;
  blog["tag_id"] = 2;
  blog["create_time"] = "2019/7/27 10:47";
  Json::Value blog_out;
  ret = blog_table.Update(blog_out);
  std::cout << "Update" << ret << std::endl;
  ret = blog_table.SelectOne(1, &blog_out);
  std::cout << "SelectOne" << ret << std::endl << writer.write(blog_out) << std::endl;
  
  std::cout << "=================================================测试删除=========================================" << std::endl;
  int blog_id = 6;
  blog_table.Delete(blog_id);
  std::cout << "Delete" << std::endl;

  MySQLRelease(mysql);
}

