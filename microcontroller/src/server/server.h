#ifndef SERVER_H
#define SERVER_H

#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>
#include <gizmo/http.h>

// Standard API responses and CORS checks come from gizmo-common
using gizmo::ensureCorsAllowed;
using gizmo::sendCorsPreflightResponse;
using gizmo::sendErrorResponse;
using gizmo::sendSuccessResponse;

// Web server shared by the web app, its API and the Wi-Fi setup
extern AsyncWebServer server;

// Web server routes (the Wi-Fi setup routes are registered by gizmo-common)
void setupStateGetRoute();
void setupSettingsPostRoute();
void setupFrameGetRoute();
void setupWebAppRoute();

// Build the state the web app shows: the settings, the scenes and what the matrix is doing
JsonDocument buildStateData();

// Register the routes and start the server (after the Wi-Fi setup registered its own)
void setupServer();

#endif