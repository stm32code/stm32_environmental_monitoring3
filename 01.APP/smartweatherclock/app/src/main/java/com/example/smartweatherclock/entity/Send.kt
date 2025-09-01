package com.example.smartweatherclock.entity

data class Send(
    var cmd: Int,
    var time: String? = null, // 时间 yyyymmddHH:mm:ssweek
    var time1: String? = null, // HHmm
    var time2: String? = null,
    var city: String? = null,
)
