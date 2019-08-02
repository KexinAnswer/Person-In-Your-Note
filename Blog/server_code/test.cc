#include "httplib.h"
int main()
{
  using namespace httplib;
  Server server;
  server.Get("/", [](const Request &req, Response &resp) {
    (void)req;
    resp.set_content("<html>hello</html>", "text/html");
  });
  server.set_base_dir("./wwwroot");
  server.listen("0.0.0.0", 9092);
  return 0;
}
return;