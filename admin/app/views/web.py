from app.views.ViewsBase import *
from app.models import *
from django.shortcuts import render,redirect
from django.contrib.auth.models import User
from app.utils.Utils import validate_email, validate_tel,gen_random_code_s
from app.utils.Common import buildPageLabels

def web_index(request):
    context = {

    }

    return render(request, 'app/web_index.html', context)

def web_profile(request):
    context = {

    }
    if request.method == 'POST':
        code = 0
        msg = "error"

        params = parse_post_params(request)

        username = params.get("username")
        email = params.get("email")
        old_password = params.get("old_password")
        new_password = params.get("new_password")
        if(len(new_password)<6 or len(new_password)>16):
            context["top_msg"] = "新密码的长度需满足6-16位"
        else:
            user = User.objects.get(username=username)

            if user and user.check_password(old_password):
                user.set_password(new_password)
                user.email = email
                user.save()
                context["top_msg"] = "修改成功"
            else:
                context["top_msg"] = "原密码验证失败"

    user = request.session[g_session_key_user]
    context["user"] = user

    print(user)

    return render(request, 'app/web_profile.html', context)

def web_logout(request):
    if request.session.has_key(g_session_key_user):
        del request.session[g_session_key_user]

    return redirect("/")


def web_login(request):
    context = {

    }

    if request.method == 'POST':
        code = 0
        msg = "error"

        params = parse_post_params(request)

        username = params.get("username")
        password = params.get("password")

        context["username"] = username
        context["password"] = password

        if validate_email(username):
            try:
                user = User.objects.get(email=username)
            except:
                user = None
            if not user:
                msg = "邮箱未注册"
        else:
            user = User.objects.get(username=username)
            if not user:
                msg = "用户名未注册"
        if user:
            if user.check_password(password):
                if user.is_active:
                    user.last_login = datetime.now()
                    user.save()

                    request.session[g_session_key_user] = {
                        "id": user.id,
                        "username": username,
                        "email": user.email,
                        "last_login": user.last_login.strftime("%Y-%m-%d %H:%M:%S")
                    }
                    code = 1000
                    msg = "登录成功"
                else:
                    msg = "账号已禁用"
            else:
                msg = "密码错误"

        res = {
            "code": code,
            "msg": msg
        }
        return HttpResponseJson(res)

    return render(request, 'app/web_login.html',context)