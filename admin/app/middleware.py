from django.http import HttpResponseRedirect, HttpResponse

try:
    from django.utils.deprecation import MiddlewareMixin  # Django 1.10.x
except ImportError:
    MiddlewareMixin = object  # Django 1.4.x - Django 1.9.x

class SimpleMiddleware(MiddlewareMixin):
    def process_request(self, request):
        path = request.path_info.lstrip('/')
        # print("process_request:path=%s" % path, request.session.keys(), request.session.get("user"))
        if request.session.has_key("user"):
            request.session["user"] = request.session["user"]
            if path.startswith("login"):
                return HttpResponseRedirect("/")
            else:
                return None
        else:
            if path.startswith("login") \
                    or path.startswith("api/postAddAlarm"):
                # 未登录状态下，需要放开的路由
                return None
            else:
                return HttpResponseRedirect("/login")

    def process_response(self, request, response):
        # print("process_response")
        return response
