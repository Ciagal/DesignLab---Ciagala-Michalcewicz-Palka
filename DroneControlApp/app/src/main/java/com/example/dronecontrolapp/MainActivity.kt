package com.example.dronecontrolapp

import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.compose.foundation.background
import androidx.compose.foundation.layout.*
import androidx.compose.material3.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.unit.dp
import androidx.compose.runtime.Composable
import androidx.compose.ui.tooling.preview.Preview
import com.example.dronecontrolapp.ui.theme.DroneControlAppTheme

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
            .background(Color.Black)  // Czarny kolor tła
            .padding(16.dp),
        horizontalArrangement = Arrangement.SpaceEvenly,
        verticalAlignment = Alignment.CenterVertically
    ) {
        // Kolumna po lewej stronie: Przód, Lewo, Prawo, Tył
        Column(
            modifier = Modifier.weight(1f),
            horizontalAlignment = Alignment.CenterHorizontally,
            verticalArrangement = Arrangement.spacedBy(16.dp)
        ) {
            Button(
                onClick = { /* Dron przesuwa się do przodu */ },
                colors = ButtonDefaults.buttonColors(containerColor = Color.Red)  // Ustawienie koloru na czerwony
            ) {
                Text("Forward", color = Color.White)
            }
            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.SpaceEvenly
            ) {
                Button(
                    onClick = { /* Dron przesuwa się w lewo */ },
                    colors = ButtonDefaults.buttonColors(containerColor = Color.Red)
                ) {
                    Text("Left", color = Color.White)
                }
                Button(
                    onClick = { /* Dron przesuwa się w prawo */ },
                    colors = ButtonDefaults.buttonColors(containerColor = Color.Red)
                ) {
                    Text("Right", color = Color.White)
                }
            }
            Button(
                onClick = { /* Dron przesuwa się do tyłu */ },
                colors = ButtonDefaults.buttonColors(containerColor = Color.Red)
            ) {
                Text("Backwards", color = Color.White)
            }
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

        // Kolumna po prawej stronie: Góra i Dół (unoszenie się)
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

@Preview(showBackground = true, widthDp = 640, heightDp = 360)
@Composable
fun DroneControlCustomPreview() {
    DroneControlAppTheme {
        DroneControlCustomLayout()
    }
}