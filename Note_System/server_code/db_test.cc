#include "db.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <jsoncpp/json/json.h>

// 使用一个程序来测试刚才封装的 MySQL 操作是否正确
// 单元测试
// gtest 谷歌提供的单元测试框架

void TestBlogTable(){
  bool ret = false;
  // 更友好的显示 Json
  Json::StyledWriter writer;
  MYSQL* mysql = MySQLInit();

  Json::Value blog;
  blog["title"] = "鸡汤";
  std::string content;
  //FileUtil::ReadFile("./test_data/1.md", &content);
  blog["content"] = "每天早睡早起身体好";
  //blog["content"] = content;
  blog["tag_id"] = 2;
  blog["create_time"] = "2019/8/1 07:35";
  std::cout << writer.write(blog) << std::endl;
//  std::cout << "=================================================测试插入==========================================" << std::endl;
//  BlogTable blog_table(mysql);
//  ret = blog_table.Insert(blog);
//  std::cout << "Insert" << ret << std::endl;
//
 //std::cout << "=================================================测试查找==========================================" << std::endl;
 //BlogTable blog_table(mysql);
 //Json::Value blogs;
 //ret = blog_table.SelectAll(&blogs);
 //std::cout << "selectAll:" << ret << std::endl << writer.write(blogs) << std::endl;
 
  //std::cout << "=================================================测试更新==========================================" << std::endl;
  //BlogTable blog_table(mysql);
  //Json::Value blog;
  //blog["blog_id"] = 1;
  //blog["title"]  = "测试更新博客";
  //blog["content"] = "1, 变量和类型\n 什么是'变量'";
  //blog["tag_id"] = 2;
  //blog["create_time"] = "2019/7/27 10:47";
  //Json::Value blog_out;
  //ret = blog_table.Update(blog);
  //std::cout << "Update" << ret << std::endl;
  //ret = blog_table.SelectOne(1, &blog_out);
  //std::cout << "SelectOne" << ret << std::endl << writer.write(blog_out) << std::endl;
  
//  std::cout << "=================================================测试删除=========================================" << std::endl;
//  BlogTable blog_table(mysql);
//  int blog_id = 1;
//  blog_table.Delete(blog_id);
//  std::cout << "Delete" << std::endl;
//
//  MySQLRelease(mysql);
//}
//
}
void TestTagTable(){
  bool ret = false;
  Json::StyledWriter writer;
  MYSQL* mysql = MySQLInit();
  TagTable tag_table(mysql);

  //std::cout << "========================================================测试插入====================================" << std::endl;
  //Json::Value tag;
  //tag["tag_name"] = "Java";
  //ret = tag_table.Insert(tag);
  //std::cout << "Insert:" << ret << std::endl;
  std::cout << "========================================================测试查看===================================" << std::endl;
  Json::Value tags;
  ret = tag_table.SelectAll(&tags);
  std::cout << "SelectAll: " << ret << std::endl << writer.write(tags) << std::endl;
  //std::cout << "========================================================测试删除===================================" << std::endl;
 // ret = tag_table.Delete(1);
 // std::cout << "Delete: " << ret << std::endl;
 // MySQLRelease(mysql);

}



void TestPasswd(){
  bool ret = false;
  Json::StyledWriter writer;
  MYSQL* mysql = MySQLInit();
  Passwd passwd(mysql);

  std::cout << "========================================================测试查看===================================" << std::endl;
  Json::Value pwd;
  ret = passwd.Select(&pwd,"UnglyBoy");
  std::cout << "Select Passwd" << ret << std::endl << writer.write(pwd) << std::endl;

}

int main(){
  // TestBlogTable();
  // TestTagTable();
  TestPasswd();
  return 0;
}