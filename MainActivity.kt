package com.example.dronecontrolapp

import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.compose.foundation.background
import androidx.compose.foundation.gestures.detectDragGestures
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.material3.*
import androidx.compose.ui.tooling.preview.Preview
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.input.pointer.pointerInput
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.unit.dp
import com.example.dronecontrolapp.ui.theme.DroneControlAppTheme
import androidx.compose.ui.geometry.Offset

class MainActivity : ComponentActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContent {
            DroneControlAppTheme {
                DroneControlCustomLayout()
            }
        }
    }
}

@Composable
fun DroneControlCustomLayout() {
    Row(
        modifier = Modifier
            .fillMaxSize()
            .background(Color.Black)
            .padding(16.dp),
        horizontalArrangement = Arrangement.SpaceEvenly,
        verticalAlignment = Alignment.CenterVertically
    ) {
        // Kolumna po lewej stronie: Analog
        Column(
            modifier = Modifier.weight(1f),
            horizontalAlignment = Alignment.CenterHorizontally,
            verticalArrangement = Arrangement.spacedBy(16.dp)
        ) {
            AnalogControl(
                modifier = Modifier
                    .size(150.dp)
                    .background(Color.DarkGray, CircleShape)
            )
        }

        // Kolumna na środku: Włącz i Wyłącz
        Column(
            modifier = Modifier.weight(1f),
            horizontalAlignment = Alignment.CenterHorizontally,
            verticalArrangement = Arrangement.spacedBy(16.dp)
        ) {
            Button(
                onClick = { /* Włączanie drona */ },
                colors = ButtonDefaults.buttonColors(containerColor = Color.Red)
            ) {
                Text("On", color = Color.White)
            }
            Button(
                onClick = { /* Wyłączanie drona */ },
                colors = ButtonDefaults.buttonColors(containerColor = Color.Red)
            ) {
                Text("Off", color = Color.White)
            }
        }

        // Kolumna po prawej stronie: Góra i Dół
        Column(
            modifier = Modifier.weight(1f),
            horizontalAlignment = Alignment.CenterHorizontally,
            verticalArrangement = Arrangement.spacedBy(16.dp)
        ) {
            Button(
                onClick = { /* Dron unosi się do góry */ },
                colors = ButtonDefaults.buttonColors(containerColor = Color.Red)
            ) {
                Text("Up", color = Color.White)
            }
            Button(
                onClick = { /* Dron obniża się w dół */ },
                colors = ButtonDefaults.buttonColors(containerColor = Color.Red)
            ) {
                Text("Down", color = Color.White)
            }
        }
    }
}

@Composable
fun AnalogControl(modifier: Modifier = Modifier) {
    var direction by remember { mutableStateOf("Idle") }

    Box(
        modifier = modifier
            .pointerInput(Unit) {
                detectDragGestures { change, dragAmount: Offset ->
                    change.consume() // Zużycie gestu
                    direction = when {
                        dragAmount.x > 0 && dragAmount.y in -50f..50f -> "Right"
                        dragAmount.x < 0 && dragAmount.y in -50f..50f -> "Left"
                        dragAmount.y > 0 && dragAmount.x in -50f..50f -> "Down"
                        dragAmount.y < 0 && dragAmount.x in -50f..50f -> "Up"
                        else -> "Idle"
                    }
                }
            },
        contentAlignment = Alignment.Center
    ) {
        Text(
            text = direction,
            color = Color.White,
            textAlign = TextAlign.Center
        )
    }
}

@Preview(showBackground = true, widthDp = 640, heightDp = 360)
@Composable
fun DroneControlCustomPreview() {
    DroneControlAppTheme {
        DroneControlCustomLayout()
    }
}
