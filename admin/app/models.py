from django.db import models
from django.utils import timezone

class Alarm(models.Model):
    sort = models.IntegerField(verbose_name='排序')
    control_code = models.CharField(max_length=50, verbose_name='布控编号')
    desc = models.CharField(max_length=100, verbose_name='描述')
    video_path = models.CharField(max_length=200, verbose_name='视频存储路径')
    image_path = models.CharField(max_length=200, verbose_name='主图存储路径')
    create_time = models.DateTimeField(auto_now_add=True,verbose_name='创建时间')
    state = models.IntegerField(verbose_name='状态') # 0 未读

    def __repr__(self):
        return self.desc

    def __str__(self):
        return self.desc

    class Meta:
        db_table = 'av_alarm'
        verbose_name = '报警视频'
        verbose_name_plural = '报警视频'

class Stream(models.Model):
    user_id = models.IntegerField(verbose_name='用户')
    sort = models.IntegerField(verbose_name='排序')
    code = models.CharField(max_length=50, verbose_name='编号')
    app = models.CharField(max_length=50, verbose_name='分组')
    name = models.CharField(max_length=50, verbose_name='名称')
    pull_stream_url = models.CharField(max_length=300, verbose_name='视频流来源')
    pull_stream_type = models.IntegerField(verbose_name='视频流来源类型')
    nickname = models.CharField(max_length=200, verbose_name='视频流昵称')
    remark = models.CharField(max_length=200, verbose_name='备注')
    forward_state = models.IntegerField(verbose_name='转发状态')  # 默认0, 0:未转发 1:转发中
    create_time = models.DateTimeField(auto_now_add=True, verbose_name='创建时间')
    last_update_time = models.DateTimeField(auto_now_add=True, verbose_name='更新时间')
    state = models.IntegerField(verbose_name='状态')

    def __repr__(self):
        return self.name

    def __str__(self):
        return self.name

    class Meta:
        db_table = 'av_stream'
        verbose_name = '视频流'
        verbose_name_plural = '视频流'



class Control(models.Model):
    user_id = models.IntegerField(verbose_name='用户')
    sort = models.IntegerField(verbose_name='排序')
    code = models.CharField(max_length=50, verbose_name='编号')

    stream_app = models.CharField(max_length=50, verbose_name='视频流应用')
    stream_name = models.CharField(max_length=100, verbose_name='视频流名称')
    stream_video = models.CharField(max_length=100, verbose_name='视频流视频')
    stream_audio = models.CharField(max_length=100, verbose_name='视频流音频')

    algorithm_code = models.CharField(max_length=50, verbose_name='算法编号')
    object_code = models.CharField(max_length=50, verbose_name='目标编号')
    polygon = models.CharField(max_length=50, verbose_name='绘制区域坐标点') # x1,y1,x2,y2,x3,y3,x4,y4
    min_interval = models.IntegerField(verbose_name='检测间隔(秒)')
    class_thresh = models.FloatField(verbose_name='分类阈值')
    overlap_thresh = models.FloatField(verbose_name='iou阈值')
    remark = models.CharField(max_length=200, verbose_name='备注')

    push_stream = models.BooleanField(verbose_name='是否推流')
    push_stream_app = models.CharField(max_length=50, null=True,verbose_name='推流应用')
    push_stream_name = models.CharField(max_length=100, null=True,verbose_name='推流名称')

    state = models.IntegerField(default=0,verbose_name="布控状态") # 0：未布控  1：布控中  5：布控中断

    create_time = models.DateTimeField(auto_now_add=True,verbose_name='创建时间')
    last_update_time = models.DateTimeField(auto_now_add=True,verbose_name='更新时间')

    def __repr__(self):
        return self.code

    def __str__(self):
        return self.code

    class Meta:
        db_table = 'av_control'
        verbose_name = '布控'
        verbose_name_plural = '布控'
