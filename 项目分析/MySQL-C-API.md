



# Person In Your Note

## 数据库API

使用 MySQL C API 来完成数据库客操作

核心就是拼接sql语句

通过Json的方式操作相关参数

### 连接数据库

首先用[mysql_init()](https://dev.mysql.com/doc/refman/5.7/en/my-init.html)创建一个句柄

```c
MYSQL* connect_fd = mysql_init(NULL);
```

使用[mysql_real_connect](https://dev.mysql.com/doc/refman/8.0/en/mysql-real-connect.html)与数据库进行连接

```c
mysql_real_connect(句柄, "主机IP", "数据库用户名", "数据库密码","数据库名(database)", 端口号(数据库默认端口号3306), unix_sock(NULL), client_flag(0));
```

```c
// 设计编码格式 为 utf8
mysql_set_character_set(connect_fd, "utf8");
```

连接成功，返回第一个参数的值，

连接失败返回NULL。

### 插入备忘录

数据库插入语句:

```sql
insert into note_table values(
	null,title, connect, tag_id,create_time
);
```

为防止文章中有特殊符号使用[mysql_real_escape_string](https://dev.mysql.com/doc/refman/8.0/en/mysql-real-escape-string.html)对connect进行字符转义

You must allocate the `to` buffer to be at least `length*2+1` bytes long. 

[mysql_query](https://dev.mysql.com/doc/refman/8.0/en/mysql-query.html)执行sql语句

向插入的结果写入 Json 对象

```c
bool Insert(const Json::Value& blog){
      // 如果拿一个文章做实验，会发现：
      // 由于文章内容中可能会包含一些特殊字符(\n, '', ""等),会导致拼装处的 sql 语句有问题
      // 应该使用 mysql_real_escape_string 对 content 字段来进行转义
      // 转义只是为了让 SQL 语句拼接正确，实际上插入成功后数据库的内容已经自动转义回来了
      const std::string& content = blog["content"].asString();
      // 文档上要求战役的缓冲区长度必须是执勤的 2 倍 + 1
      // 为了防止内存泄漏 不需要指针拷贝和指针赋值
      // 使用 unique_ptr 管理内存
      std::unique_ptr<char> content_escape(new char[content.size() * 2 + 1]);
      mysql_real_escape_string(mysql_, content_escape.get(), content.c_str(), content.size());
      // 插入的博客时间可能较长， 需要搞个大一点的缓冲区(根据用户请求的长度自适应)
      std::unique_ptr<char> sql(new char[content.size() * 2 + 4096]);
      sprintf(sql.get(), "insert into note_table values(null, '%s', '%s' , %d, '%s')", 
          blog["title"].asCString(), 
          content_escape.get(), 
          blog["tag_id"].asInt(),
          blog["create_time"].asCString());
      // 执行sql语句
      int ret = mysql_query(mysql_,sql.get());
      printf("sql:%s\n", sql.get());
      if(ret != 0){
        printf("执行 sql 失败! sql = %s, %s\n", sql.get(), mysql_error(mysql_));
        return false;
      }
      printf("执行插入文章成功!");
      return true;
    }
```

### 查找备忘录(全部查找)

数据库插入语句:

```sql
select note_id, title, tag_id, create_time from note_table;
```

[mysql_query](https://dev.mysql.com/doc/refman/8.0/en/mysql-query.html)执行sql语句

[mysql_store_result](https://dev.mysql.com/doc/refman/8.0/en/mysql-store-result.html)获取执行结果

[mysql_num_rows](https://dev.mysql.com/doc/refman/8.0/en/mysql-num-rows.html) 将查询结果分成行

[mysql_fecth_row](https://dev.mysql.com/doc/refman/8.0/en/mysql-fetch-row.html) 获取每一行的信息，每次获取下一行

将获取结果写入Json对象中

```c
 bool SelectAll(Json::Value* blogs){
      char sql[1024 * 4] = {0};
      // 可以根据 tag_id 来筛选结果
      sprintf(sql,"select note_id, title, tag_id, create_time from note_table");
      // 执行sql语句, 参数mysql句柄 sql语句
      int ret = mysql_query(mysql_,sql);
      if( ret != 0){
        printf("执行 sql 失败!%s \n", mysql_error(mysql_));
        return false;
      }
      // 获取查询结果
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
        blog["note_id"] = atoi(row[0]);
        blog["title"] = row[1];
        blog["tag_id"] = atoi(row[2]);
        blog["create_time"] = row[3];
        // 遍历结果依次加入到 dishes 中
        printf("%d\n %s\n %d\n %s\n", atoi(row[0]), row[1], atoi(row[2]) , row[3]);
        printf("blogs->append(blog)\n");
        blogs->append(blog);
      }
      // mysql 查询的结果集后需要记得释放
      mysql_free_result(result);
      printf("执行查找文章功!, 共查找到 %d 条文章\n" , rows);
      return true;
    
```

### 查找备忘录(单个查找)

```sql
select note_id, title,content, tag_id, create_time from note_table where note_id = 1;
```

[mysql_query](https://dev.mysql.com/doc/refman/8.0/en/mysql-query.html)执行sql语句

[mysql_store_result](https://dev.mysql.com/doc/refman/8.0/en/mysql-store-result.html)获取执行结果

[mysql_num_rows](https://dev.mysql.com/doc/refman/8.0/en/mysql-num-rows.html) 将查询结果分成行

将获取结果写入Json对象中

```c
bool SelectOne(int32_t note_id, Json::Value* blog){
      char sql[1024 * 4] = {0}; 
      sprintf(sql,"select note_id, title,content, tag_id, create_time from note_table where note_id = = %d", note_id);
     // 打印sql语句 查看sql语句是否正确
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
```

### 更新备忘录

```sql
update note_table SET title='更改标题', content='更改标题内容',  tag_id=1 where note_id=1
```

为防止文章中有特殊符号使用[mysql_real_escape_string](https://dev.mysql.com/doc/refman/8.0/en/mysql-real-escape-string.html)对connect进行字符转义

You must allocate the `to` buffer to be at least `length*2+1` bytes long. 

[mysql_query](https://dev.mysql.com/doc/refman/8.0/en/mysql-query.html)执行sql语句

```c
bool Update(const Json::Value& blog){
      // 由于文章内容中柯南包含一些特殊字符(\n, '', ""等), 会导致拼接出来的 sql 语句有问题
      // 应该使用 mysql_query_espace_string 对 content 字段进行转义
      // 转义只是为了让 SQL 语句拼接正确，实际上插入成功后的数据库的内容已经自动转义回来了
      const std::string& content = blog["content"].asString();
      // 文档上要求转义的缓冲区长度必须是之前的 2 倍 + 1
      // 使用 unique_ptr 管理内存
      std::unique_ptr<char> content_espace(new char[content.size() * 2 + 1]);
      mysql_real_escape_string(mysql_, content_espace.get(), content.c_str(), content.size());
      // 更新博客的内容可能较长,需要搞一个大点的缓冲区(根据用户请求的长度自适应)
      std::unique_ptr<char> sql(new char[content.size() * 2 + 4096]);
      sprintf(sql.get(),"update note_table SET title='%s', content='%s',  tag_id=%d where note_id=%d",
          blog["title"].asCString(),
          content_espace.get(),
          blog["tag_id"].asInt(),
          blog["note_id"].asInt());

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
```

### 删除备忘录

```sql
delete from note_table where note_id=1;
```

[mysql_query](https://dev.mysql.com/doc/refman/8.0/en/mysql-query.html)执行sql语句

```c
    bool Delete(int32_t blog_id){
      char sql[1024]  = {0};
      sprintf(sql,"delete from note_table where note_id=%d", note_id);
      int ret = mysql_query(mysql_, sql);
      if(ret != 0){
        printf("执行 sql 失败! sql=%s, %s\n", sql,mysql_error(mysql_));
        return false;
      }
      return true;
    }
```



### 断开数据库

最后关闭数据库

[mysql_close](https://dev.mysql.com/doc/refman/8.0/en/mysql-close.html)

释放句柄

```c
mysql_close(connect_fd);
```

