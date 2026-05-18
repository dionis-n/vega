

if(EXISTS "${HTML_FILE}")

    file(READ "${HTML_FILE}" CONTENT)


    set(SEARCH_TITLE "</title>")
    set(REPLACE_TITLE "</title><link rel=\"icon\" href=\"Resources/VegaIcon/Vega.ico\"><link rel=\"manifest\" href=\"manifest.json\">")
    string(REPLACE "${SEARCH_TITLE}" "${REPLACE_TITLE}" CONTENT "${CONTENT}")


    set(SEARCH_BODY "</body>")
    set(REPLACE_BODY "<script>if (\"serviceWorker\" in navigator) { navigator.serviceWorker.register(\"service-worker.js\"); }</script></body>")
    string(REPLACE "${SEARCH_BODY}" "${REPLACE_BODY}" CONTENT "${CONTENT}")


    file(WRITE "${HTML_FILE}" "${CONTENT}")
else()
    message(WARNING "Патч не выполнен: Файл не найден по пути ${HTML_FILE}")
endif()