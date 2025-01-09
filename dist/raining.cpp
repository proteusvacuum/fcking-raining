#include "lib/json.hpp"
#include <emscripten.h>
#include <emscripten/bind.h>
#include <emscripten/fetch.h>
#include <iostream>
#include <stdio.h>
#include <string.h>

using json = nlohmann::json;
using namespace emscripten;

void downloadSucceeded(emscripten_fetch_t *fetch) {
  printf("Finished downloading %llu bytes from URL %s.\n", fetch->numBytes,
         fetch->url);
  std::string response(&fetch->data[0], fetch->numBytes);
  try {

    json weather_data = json::parse(response);
    bool is_raining = weather_data["weather"][0]["main"] == "Rain";
    printf(is_raining ? "YES\n" : "NO\n");

    // JAVASCRIPT!!!
    EM_ASM(
        { isItRaining(UTF8ToString($0)); },
        is_raining ? "IT'S FCKING RAINING!"
                   : "IT'S NOT FCKING RAINING... GO THE FCK OUTSIDE!");

  } catch (const json::parse_error &e) {
    std::cout << "JSON parse error: " << e.what() << std::endl;
    std::cout << "Error at byte offset " << e.byte << std::endl;
    std::cout << fetch->data << std::endl;
  }
  emscripten_fetch_close(fetch); // Free data associated with the fetch.
}

void downloadFailed(emscripten_fetch_t *fetch) {
  printf("Downloading %s failed, HTTP failure status code: %d.\n", fetch->url,
         fetch->status);
  emscripten_fetch_close(fetch); // Also free data on failure.
}

void fetch_weather(std::string lat, std::string lon) {
  std::string url = "https://api.openweathermap.org/data/2.5/"
                    "weather?lat=" +
                    lat + "&lon=" + lon +
                    "&appid=b94679f30100fd559bad3b7a1612cee3";
  emscripten_fetch_attr_t attr;
  emscripten_fetch_attr_init(&attr);
  strcpy(attr.requestMethod, "GET");
  attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;
  attr.onsuccess = downloadSucceeded;
  attr.onerror = downloadFailed;
  emscripten_fetch(&attr, url.c_str());
}

int main() {
  std::string lat = "-0.780";
  std::string lon = "-52.425";
  // fetch_weather(lat, lon);

  return 0;
}

EMSCRIPTEN_BINDINGS(my_module) { function("fetch_weather", &fetch_weather); }
