<ul class="am-nav am-nav-pills">
    <li class="am-active"> 🖱️<span>&nbsp</span> <a href="#">首页</a></li>
  <li>📚 <a href="https://blog.csdn.net/qq_41595735/article/details/98639866">项目分析</a></li>
  <li>❤️ <a href="http://47.95.141.253:9092/index.html">作品展示</a>
</li>
</ul>

# Person In Your Note

基于C++的网页版备忘录系统

## 开发环境和编程语言

- Linux
- g++

## 功能介绍

- 用户界面
  - 展示备忘录列表
  - 查看备忘录
  - 显示备忘录标题
- 管理员界面
  - 新增备忘录(备忘录格式为Markdown)
  - 删除备忘录
  - 编辑备忘录

## 模块划分

- MySQL存储数据
  - 存储文章信息(文章的标题、正文、发表时间)
  - 存储标签信息(标签名)对文章进行分类
- 封装服务器接口

  - 用 MySQL C API 封装接口 实现对文章和标签的增删查该功能
- HTML 进行前端展示
  - 展示页面，向服务器发送请求

## 项目测试

- gtest 进行单元测试
- Postman 检测请求和响应是否正确
- 每篇备忘录文章最大可存储 64Ｋ大小

## 待开发模块

- 图床系统（用户可以上传图片）
- 多用户系统（每个用于可以有独立的隐私备忘录）
- 提醒功能(备忘录中可以设置提醒时间)
- 未来邮件功能(备忘录可以以邮件的方式选定未来的时间发送给某人)