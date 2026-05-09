#include "MainWidget.h"
#include <QApplication>
#include <emscripten.h>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    app.setOrganizationDomain("https://github.com/danila258/Vega");
    app.setApplicationName("Vega");

    EM_ASM(
        var link = document.createElement('link');
        link.rel = 'manifest';
        link.href = 'manifest.json';
        document.head.appendChild(link);

        if ('serviceWorker' in navigator) {
            navigator.serviceWorker.register('service-worker.js');
        }
        );

    MainWidget* Vega = new MainWidget();
    Vega->show();

    return app.exec();
}