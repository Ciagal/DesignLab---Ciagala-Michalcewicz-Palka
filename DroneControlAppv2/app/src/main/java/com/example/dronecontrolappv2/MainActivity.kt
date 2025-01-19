package com.example.dronecontrolappv2

import android.os.Bundle
import android.view.MotionEvent
import android.view.View
import android.widget.Button
import android.widget.RelativeLayout
import android.widget.TextView
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity
import okhttp3.*
import okhttp3.RequestBody.Companion.toRequestBody
import java.io.IOException

class MainActivity : AppCompatActivity() {

    private val client = OkHttpClient()

    private var isArmed = false

    // SBUS kanały
    private var rollValue = 0
    private var pitchValue = 0
    private var throttleValue = 1000  // Domyślnie minimalna przepustnica = 1000
    private var yawValue = 1500
    private var armChannelValue = 1000

    // Throttle – pointer
    private var throttlePointerId = -1
    private var initialY = 0f

    // Joystick – pointer
    private var joystickPointerId = -1
    private var initialXJoystick = 0f
    private var initialYJoystick = 0f

    private var lastSentTime = 0L

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        val btnArm: Button = findViewById(R.id.btnArm)
        val btnDisarm: Button = findViewById(R.id.btnDisarm)
        val btnStop: Button = findViewById(R.id.btnStop)

        val throttleControl: RelativeLayout = findViewById(R.id.throttleControl)
        val centerDot: View = findViewById(R.id.centerDot)

        val joystickControl: RelativeLayout = findViewById(R.id.joystickControl)
        val joystickDot: View = findViewById(R.id.joystickDot)

        val tvThrottle: TextView = findViewById(R.id.tvGasValue)
        val tvJoystickValues: TextView = findViewById(R.id.tvJoystickValues)
        val tvArmDisarmStatus: TextView = findViewById(R.id.tvArmDisarmStatus)

        // ----------------------------------
        // Obsługa przycisków Arm / Disarm / Stop
        // ----------------------------------

        // ARMWANIE
        btnArm.setOnClickListener {
            isArmed = true
            armChannelValue = 2000
            // Ustawiamy minimalny gaz:
            throttleValue = 1000
            tvArmDisarmStatus.text = "Status: Armed"
            tvArmDisarmStatus.setTextColor(getColor(android.R.color.holo_green_dark))
            sendAllChannels()
        }

        // ROZBROJENIE
        btnDisarm.setOnClickListener {
            isArmed = false
            armChannelValue = 1000
            throttleValue = 1000
            rollValue = 0
            pitchValue = 0
            yawValue = 1500
            tvArmDisarmStatus.text = "Status: Disarmed"
            tvArmDisarmStatus.setTextColor(getColor(android.R.color.holo_red_dark))
            sendAllChannels()
        }

        // STOP
        btnStop.setOnClickListener {
            isArmed = false
            armChannelValue = 1000
            throttleValue = 1000
            rollValue = 0
            pitchValue = 0
            yawValue = 1500
            sendAllChannels()
            Toast.makeText(this, "Emergency STOP!", Toast.LENGTH_SHORT).show()
        }

        // -----------------------------
        // THROTTLE – onTouch
        // -----------------------------
        centerDot.setOnTouchListener { v, event ->
            if (!isArmed) return@setOnTouchListener true

            v.parent.requestDisallowInterceptTouchEvent(true)

            when (event.actionMasked) {
                MotionEvent.ACTION_DOWN -> {
                    throttlePointerId = event.getPointerId(event.actionIndex)
                    initialY = event.getY(event.actionIndex) - v.y
                }
                MotionEvent.ACTION_POINTER_DOWN -> {
                    if (throttlePointerId == -1) {
                        throttlePointerId = event.getPointerId(event.actionIndex)
                        initialY = event.getY(event.actionIndex) - v.y
                    }
                }
                MotionEvent.ACTION_MOVE -> {
                    val pointerIndex = event.findPointerIndex(throttlePointerId)
                    if (pointerIndex != -1 && throttlePointerId != -1) {
                        val newY = (event.getY(pointerIndex) - initialY)
                            .coerceIn(0f, throttleControl.height - v.height.toFloat())

                        v.y = newY
                        // Mapa [0..(height-dot)] => [1000..2000]
                        throttleValue = (1000 +
                                ((throttleControl.height - newY) / throttleControl.height) * 1000).toInt()

                        // W razie potrzeby daj tutaj coerceIn(990, 2000), żeby mieć zapas
                        throttleValue = throttleValue.coerceIn(1000, 2000)

                        tvThrottle.text = "Throttle: $throttleValue"
                        sendAllChannels()
                    }
                }
                MotionEvent.ACTION_UP, MotionEvent.ACTION_POINTER_UP -> {
                    val upPointerId = event.getPointerId(event.actionIndex)
                    if (upPointerId == throttlePointerId) {
                        throttlePointerId = -1
                    }
                }
            }
            true
        }

        // -----------------------------
        // JOYSTICK (Roll/Pitch) – onTouch
        // -----------------------------
        joystickDot.setOnTouchListener { v, event ->
            if (!isArmed) return@setOnTouchListener true

            v.parent.requestDisallowInterceptTouchEvent(true)

            when (event.actionMasked) {
                MotionEvent.ACTION_DOWN -> {
                    joystickPointerId = event.getPointerId(event.actionIndex)
                    initialXJoystick = event.getX(event.actionIndex) - v.x
                    initialYJoystick = event.getY(event.actionIndex) - v.y
                }
                MotionEvent.ACTION_POINTER_DOWN -> {
                    if (joystickPointerId == -1) {
                        joystickPointerId = event.getPointerId(event.actionIndex)
                        initialXJoystick = event.getX(event.actionIndex) - v.x
                        initialYJoystick = event.getY(event.actionIndex) - v.y
                    }
                }
                MotionEvent.ACTION_MOVE -> {
                    val pointerIndex = event.findPointerIndex(joystickPointerId)
                    if (pointerIndex != -1 && joystickPointerId != -1) {
                        val maxX = joystickControl.width - v.width
                        val maxY = joystickControl.height - v.height

                        val newX = (event.getX(pointerIndex) - initialXJoystick)
                            .coerceIn(0f, maxX.toFloat())
                        val newY = (event.getY(pointerIndex) - initialYJoystick)
                            .coerceIn(0f, maxY.toFloat())

                        v.x = newX
                        v.y = newY

                        rollValue = ((newX / maxX) * 200 - 100).toInt()
                        pitchValue = -((newY / maxY) * 200 - 100).toInt()

                        tvJoystickValues.text = "Roll: $rollValue, Pitch: $pitchValue"
                        sendAllChannels()
                    }
                }
                MotionEvent.ACTION_UP, MotionEvent.ACTION_POINTER_UP -> {
                    val upPointerId = event.getPointerId(event.actionIndex)
                    if (upPointerId == joystickPointerId) {
                        joystickPointerId = -1
                    }
                }
            }
            true
        }
    }

    private fun sendAllChannels() {
        // Debounce co 100 ms
        if (System.currentTimeMillis() - lastSentTime > 100) {
            val rollSbus = 1500 + rollValue * 5
            val pitchSbus = 1500 + pitchValue * 5

            val command = """
                CH1=$rollSbus
                CH2=$pitchSbus
                CH3=$throttleValue
                CH4=$yawValue
                CH5=$armChannelValue
            """.trimIndent() + "\n"

            sendCommandToESP32(command)
            lastSentTime = System.currentTimeMillis()
        }
    }

    private fun sendCommandToESP32(command: String) {
        val url = "http://192.168.4.1/"
        val requestBody = command.toRequestBody()
        val request = Request.Builder()
            .url(url)
            .post(requestBody)
            .build()

        client.newCall(request).enqueue(object : Callback {
            override fun onFailure(call: Call, e: IOException) {
                runOnUiThread {
                    Toast.makeText(
                        this@MainActivity,
                        "Error: ${e.message}",
                        Toast.LENGTH_SHORT
                    ).show()
                }
            }

            override fun onResponse(call: Call, response: Response) {
                val responseBody = response.body?.string() ?: ""
                runOnUiThread {
                    Toast.makeText(
                        this@MainActivity,
                        "Command Sent:\n$command\nResponse:\n$responseBody",
                        Toast.LENGTH_SHORT
                    ).show()
                }
            }
        })
    }
}