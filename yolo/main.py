import tornado.ioloop
import tornado.httpserver
import tornado.options
import tornado.web
import tornado.log
import platform
from controllers.IndexHandler import IndexHandler
from controllers.DetectHandler import DetectHandler

def Server(ip, port, debug=True):
    print("Server http://%s:%d start" % (ip, port))

    conf = dict(
        xsrf_cookies=False,
        cookie_secret="Server",
        autoreload=False,
        debug=debug
    )

    if debug:
        conf["autoreload"] = False
        num_processes = 1
    else:
        num_processes = 0

    if "Windows" == platform.system() or "windows" == platform.system():
        reuse_port = False # Windows平台不支持端口复用
    else:
        reuse_port = True

    app = tornado.web.Application([
        (r"/", IndexHandler),
        (r"/algorithm", DetectHandler),
    ], **conf)

    max_buffer_size = 1024 * 1024 * 10 # 文件最大上传字节长度（10M）

    server = tornado.httpserver.HTTPServer(app, max_buffer_size=max_buffer_size)
    server.bind(port=port,reuse_port=reuse_port)
    server.start(num_processes=num_processes)  # tornado将按照cpu核数来fork进程，自定义num_processes需要关闭debug模式，否则会出错
    tornado.ioloop.IOLoop.instance().start()

if __name__ == "__main__":
    # tornado.options.parse_command_line()
    Server(ip="0.0.0.0", port=9702, debug=True)



