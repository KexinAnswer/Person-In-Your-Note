#include <signal.h>
#include "db.hpp"
#include "httplib.h"

MYSQL* mysql = NULL;

int main(){
  using namespace httplib;
  // 1. 想和数据库建立好连接
  // 1. 数据库客户端 初始化 和 释放
  mysql = MySQLInit();
  signal(SIGINT, [](int){
      MySQLRelease(mysql);
      exit(0);
      });
  // 2. 创建相关数据库处理对象
  BlogTable blog_table(mysql);
  TagTable tag_table(mysql);
  // 3. 创建扶服务器 , 并设置 "路由"(HTTP服务器中的路由，
  // 和 IP协议的路由不一样) 此处的路由指的是把 方法 + path ->
  // 哪个处理函数 关联相关声明
  Server server;

  // 新增博客
  server.Post("/blog", [&blog_table](const Request& req, Response resp){
      //  1. 获取到请求中的 body 并解析成功 json
      std::cout << "新增博客" << req.body << std::endl;
      Json::Reader reader;
      Json::FastWriter writer;
      Json::Value req_json;
      Json::Value resp_json;
      bool ret = reader.parse(req.body, req_json);
      //    解析出错， 给用提示 
      if(!ret) {
      printf("解析请求失败! %s\n", req.body.c_str());

      //    构造一个响应对象， 告诉客户端出错了
      resp_json["ok"] = false;
      resp_json["resson"] = "parse Request failed!\n";
      //    input data path failed  
      resp.status = 400;
      resp.set_content(writer.write(resp_json),"application/json");
      return;
      }
      std::cout << "解析成功" << std::endl;

      //  2. 对参数进行校验
      //    请求数据格式有错
      //    input data format error
      if(req_json["title"].empty() ||
          req_json["content"].empty() ||
          req_json["tag_id"].empty() ||
          req_json["create-time"].empty()){
        //    构造一个响应对象， 告诉客户端出错了
        resp_json["ok"] = false;
        resp_json["resson"] = "Request fields error!\n";
        //    input data path failed  
        resp.status = 400;
        resp.set_content(writer.write(resp_json),"application/json");
        return;  
      }
      std::cout << "校验成功" << std::endl;
      //  3. 真正的调用 MySQL 接口来操作
      ret = blog_table.Insert(req_json);
      if(!ret){
        //    构造一个响应对象， 告诉客户端出错了
        resp_json["ok"] = false;
        resp_json["resson"] = "Insert Failed!\n";
        //    input data path failed  
        resp.status = 500;
        resp.set_content(writer.write(resp_json),"application/json");
        printf("解析请求失败! %s\n", req.body.c_str());
        //    blog insert failed
        return;  
      }
      std::cout << "插入成功" << std::endl;
      //  4. 构造一个正确的响应给用户端即可
      resp_json["ok"] = true;
      resp.set_content(writer.write(resp_json), "appplication/json");
      return;
  });

  // 查看所有博客列表
  server.Get("/blog", [&blog_table](const Request& req, Response resp){
      std::cout << "查看所有博客" << std::endl;
      Json::Reader reader;
      Json::FastWriter writer;
      Json::Value resp_json;
      const std::string tag_id = req.get_param_value("tag_id");
      Json::Value blogs;
      bool ret = blog_table.SelectAll(&blogs,tag_id);
      if(!ret){
        resp_json["ok"] = false;
        resp_json["resson"] = "SelectAll Failed!\n";
        //    input data path failed  
        resp.status = 500;
        resp.set_content(writer.write(resp_json),"application/json");
        printf("解析请求失败! %s\n", req.body.c_str());
        //    blog insert failed
        return;  
      }
      resp.set_content(writer.write(blogs), "application/json");
      return;
    }); 
  //   1. 尝试获取 tag_id 如果 tag_id 这个参数不存在
  //     返回空字符串
  //
  //     就不需要解析请求了 也就不需要合法性判断了
  //   2. 调用数据库操作来获取所有博客的结果
  //   3. 构造响应结果就行了
  // 查看某个博客
  server.Get(R"(/blog/(\d+))", [&blog_table](const Request &req, Response resp) {
    Json::Value resp_json;
    Json::FastWriter writer;
    int blog_id = std::stoi(req.matches[1]);
    std::cout << "查看指定的博客" << std::endl;
    bool ret = blog_table.SelectOne(blog_id, &resp_json);
    if (!ret)
    {
      resp_json["ok"] = false;
      resp_json["resson"] = "SelectOne Failed!\n";
      //    input data path failed
      resp.status = 500;
      resp.set_content(writer.write(resp_json), "application/json");
      printf("解析请求失败! %s\n", req.body.c_str());
      //    blog insert failed
      return;
    }
    resp_json["ok"] = true;
    resp.set_content(writer.write(resp_json), "application/json");
    return;
  });
  // 修改某个博客
  server.Put(R"(/blog/(\d+))", [&blog_table](const Request &req, Response resp) {
    Json::Reader reader;
    Json::FastWriter writer;
    Json::Value req_json;
    Json::Value resp_json;
    int blog_id = std::stoi(req.matches[1]);
    std::cout << "修改博客" << std::endl;
    bool ret = reader.parse(req.body, req_json);
    if(!ret)
    {
      resp_json["ok"] = false;
      resp_json["resson"] = "parse Request Failed!\n";
      //    input data path failed
      resp.status = 400;
      resp.set_content(writer.write(resp_json), "application/json");
      return;
    }
    req_json["blog_id"] = blog_id;
    // 检查博客信息
    if (req_json["title"].empty() || req_json["content"].empty() || req_json["reason"].empty() || req_json["tag_id"].empty())
    {
      resp_json["ok"] = false;
      resp_json["resson"] = " Request has no name or price!\n";
      //    input data path failed
      resp.status = 400;
      resp.set_content(writer.write(resp_json), "application/json");
      return;
    }
    // 4. 调用数据库接口进行修改
    ret = blog_table.Update(req_json);
    if(!ret){
      resp_json["ok"] = false;
      resp_json["resson"] = "Update Failed!\n";
      resp.status = 400;
      resp.set_content(writer.write(resp_json), "application/json");
      return;
    }
    // 5. 封装正确
    req_json["ok"] = true;
    resp.set_content(writer.write(resp_json), "application/json");
    return;
  });
  // 删除博客
  server.Delete(R"(/blog/(\d+))", [&blog_table](const Request& req, Response resp){
    Json::Value resp_json;
    Json::FastWriter writer;
    int blog_id = std::stoi(req.matches[1]);
    bool ret = blog_table.Delete(blog_id);
    if(!ret){
      resp_json["ok"] = false;
      resp_json["resson"] = "Update Failed!\n";
      resp.status = 500;
      resp.set_content(writer.write(resp_json), "application/json");
      return;

    }
    resp_json["ok"] = true;
    resp.set_content(writer.write(resp_json), "application/json");
    return;
      });
  // 新增标签
  server.Post("/tag", [&tag_table](const Request& req, Response resp){
      std::cout << "新增标签" << std::endl;
    Json::Reader reader;
    Json::FastWriter writer;
    Json::Value req_json;
    Json::Value resp_json;
    bool ret = reader.parse(req.body, req_json);
    if(!ret){
      resp_json["ok"] = false;
      resp_json["resson"] = "Parse request Failed!\n";
      resp.status = 400;
      resp.set_content(writer.write(resp_json), "application/json");
      return;
    }
    // 校验信息
    if(req_json["tag_name"].empty()){
      resp_json["ok"] = false;
      resp_json["resson"] = "Request has no table_id or time or dish!\n";
      resp.status = 400;
      resp.set_content(writer.write(resp_json), "application/json");
      return;
    }
    ret = tag_table.Insert(req_json);
    if(!ret){
      resp_json["ok"] = false;
      resp_json["resson"] = "Insert Failed!\n";
      resp.status = 500;
      resp.set_content(writer.write(resp_json), "application/json");
      return;
    }
    resp_json["ok"] = true;
    resp.set_content(writer.write(resp_json), "application/json");
    return;
      });
  // 删除标签
  server.Delete(R"(/tag/(\d+))",[&tag_table](const Request& req, Response resp){
    Json::FastWriter writer;
    Json::Value req_json;
    int tag_id = std::stoi(req.matches[1]);
    bool ret = tag_table.Delete(tag_id);
    if(!ret){
      resp_json["ok"] = false;
      resp_json["resson"] = "Insert Failed!\n";
      resp.status = 500;
      resp.set_content(writer.write(resp_json), "application/json");
      return;
    }
    resp_json["ok"] = true;
    resp.set_content(writer.write(resp_json), "application/json");
    return;
      });
  // 获取所有标签
  server.Get("/tag", [&tag_table](const Request &req, Response resp) {
    Json::Value resp_json;
    Josn::FastWriter writer;
    Json::Reader reader;
    Json::Value tags;
    bool ret = tag_table.SelectAll(&tags);
    if (!ret)
    {
      resp_json["ok"] = false;
      resp_json["resson"] = "Insert Failed!\n";
      resp.status = 500;
      resp.set_content(writer.write(resp_json), "application/json");
      return;
    }
    resp_json["ok"] = true;
    resp.set_content(writer.write(resp_json), "application/json");
    return;
  });
  // 设置静态文件目录
  server.set_base_dir("./wwwroot");
  server.listen("0.0.0.0", 9093);

  return 0;
}
