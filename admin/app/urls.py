from django.urls import path
from .views.web import *
from .views.Algorithm import *
from .views.ControlView import *
from .views import AlarmView
from .views import StreamView
from .views.api import *


app_name = 'app'

urlpatterns = [
    path('', web_index),
    # 视频流功能
    path('stream/online', StreamView.online),
    path('stream/getIndex', StreamView.api_getIndex),
    path('stream/index', StreamView.index),
    path('stream/add', StreamView.add),
    path('stream/edit', StreamView.edit),
    path('stream/postDel', StreamView.api_postDel),
    path('stream/postHandleForward', StreamView.api_postHandleForward),
    path('stream/gb28181', StreamView.web_gb28181_index),
    path('stream/player', StreamView.player),
    path('stream/getOnline', StreamView.api_getOnline),
    path('stream/getAllStartForward', StreamView.api_getAllStartForward),
    path('stream/getAllUpdateForwardState', StreamView.api_getAllUpdateForwardState),


    path('alarms', AlarmView.web_alarms),
    # 算法
    path('algorithms', web_algorithms),
    # 布控
    path('controls', web_controls),
    path('control/add', web_add_control),
    path('control/edit', web_edit_control),

    path('profile', web_profile),
    path('login', web_login),
    path('logout', web_logout),

    path('api/postHandleAlarm', api_postHandleAlarm),
    path('api/postAddAlarm', api_postAddAlarm),
    path('api/postAddControl', api_postAddControl),
    path('api/postEditControl', api_postEditControl),
    path('api/postDelControl', api_postDelControl),
    path('api/postAddAnalyzer', api_postAddAnalyzer),
    path('api/postCancelAnalyzer', api_postCancelAnalyzer),
    path('api/getControls', api_getControls),
    path('api/getIndex', api_getIndex),
    path('api/getStreams', api_getStreams)
]