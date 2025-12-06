package com.jobs.app

import android.app.Application

class JobsApplication : Application() {
    companion object {
        lateinit var instance: JobsApplication
            private set
    }

    override fun onCreate() {
        super.onCreate()
        instance = this
    }
}

