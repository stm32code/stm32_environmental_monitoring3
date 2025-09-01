package com.example.smartweatherclock.http

import com.example.smartweatherclock.entity.City
import com.example.smartweatherclock.utils.Common
import retrofit2.Call
import retrofit2.http.GET
import retrofit2.http.Headers
import retrofit2.http.Path
import retrofit2.http.Query

interface HttpAPI {
//    @GET("search.json?key=${Common.WEATHER_API}&q={city}")
    @GET("search.json?key=${Common.WEATHER_API}")
    fun getCity(@Query("q") city : String): Call<City>
}
