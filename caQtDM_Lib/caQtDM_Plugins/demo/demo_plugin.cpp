#include <QDebug>
#include "demo_plugin.h"

// as defined in knobDefines.h
//caType {caSTRING	= 0, caINT = 1, caFLOAT = 2, caENUM = 3, caCHAR = 4, caLONG = 5, caDOUBLE = 6};

// this demo plugin just gives you an idea how to use a plugin; for more details you should take a look
// at the epics3 plugin

// gives the plugin name back
QString DemoPlugin::pluginName()
{
    return "demo";
}

// constructor
DemoPlugin::DemoPlugin()
{
    qDebug() << "DemoPlugin: Create";
}

// in this demo we update our interface here; normally you should update in from your controlsystem
// take a look how monitors are treated in the epics3 plugin
void DemoPlugin::updateInterface()
{
    double newValue;

    QMutexLocker locker(&mutex);

    // go through our devices
    foreach(int index, listOfIndexes) {
        knobData* kData = mutexknobdata->GetMutexKnobDataPtr(index);
        if((kData != (knobData *) 0) && (kData->index != -1)) {
            QString key = kData->pv;

            // find this pv in our internal double values list (assume for now we are only treating doubles)
            // and increment its value
            QMap<QString, double>::iterator i = listOfDoubles.find(key);
            while (i !=listOfDoubles.end() && i.key() == key) {
                newValue = i.value();
                break;
            }
            // update some data
            kData->edata.rvalue = newValue;
            kData->edata.fieldtype = caDOUBLE;
            kData->edata.connected = true;
            kData->edata.accessR = kData->edata.accessW = true;
            kData->edata.monitorCount++;
            mutexknobdata->SetMutexKnobData(kData->index, *kData);
            mutexknobdata->SetMutexKnobDataReceived(kData);
        }
    }
}

// in this demo we update our values here
void DemoPlugin::updateValues()
{
    QMutexLocker locker(&mutex);
    QMap<QString, double>::iterator i;
    for (i = listOfDoubles.begin(); i != listOfDoubles.end(); ++i) i.value()++;
}

// initialize our communicationlayer with everything you need
int DemoPlugin::initCommunicationLayer(MutexKnobData *data)
{
    qDebug() << "DemoPlugin: InitCommunicationLayer" << data;
    mutexknobdata = data;
    initValue = 0.0;

    // we want to update our internal doubles every second
    timerValues = new QTimer(this);
    connect(timerValues, SIGNAL(timeout()), this, SLOT(updateValues()));
    timerValues->start(1000);

    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(updateInterface()));
    timer->start(2000);

    return true;
}

// caQtDM_Lib will call this routine for defining a monitor
int DemoPlugin::pvAddMonitor(int index, knobData *kData, int rate, int skip) {
    QMutexLocker locker(&mutex);
    QString key = kData->pv;

    qDebug() << "DemoPlugin:pvAddMonitor" << kData->pv << kData->index;
    double value = initValue;
    initValue =+ 10;

    // append device index to our internal list
    listOfIndexes.append(kData->index);

    // initial values into the doubles list
    if(!listOfDoubles.contains(key)) listOfDoubles.insert(key, value);

    return true;
} 

// caQtDM_Lib will call this routine for getting rid of a monitor
int DemoPlugin::pvClearMonitor(knobData *kData) {
    QMutexLocker locker(&mutex);

    qDebug() << "DemoPlugin:pvClearMonitor" << kData << kData->pv << kData->index;
    QString key = kData->pv;
    if(!listOfDoubles.contains(key)) listOfDoubles.remove(key);
    listOfIndexes.removeAll(kData->index);

    return true;
}

// caQtDM_Lib will call this routine for setting data (see for more detail the epics3 plugin)
int DemoPlugin::pvSetValue(char *pv, double rdata, int32_t idata, char *sdata, char *object, char *errmess, int forceType) {
    QMutexLocker locker(&mutex);
    qDebug() << "DemoPlugin:pvSetValue" << pv << rdata << idata << sdata;
    QString key = pv;
    if(listOfDoubles.contains(key)) listOfDoubles.insert(pv, rdata);
    return true;
}

// caQtDM_Lib will call this routine for setting waveforms data (see for more detail the epics3 plugin)
int DemoPlugin::pvSetWave(char *pv, float *fdata, double *ddata, int16_t *data16, int32_t *data32, char *sdata, int nelm, char *object, char *errmess) {
    QMutexLocker locker(&mutex);
    qDebug() << "DemoPlugin:pvSetWave";
    return true;
}

// caQtDM_Lib will call this routine for getting a description of the monitor
int DemoPlugin::pvGetTimeStamp(char *pv, char *timestamp) {
    qDebug() << "DemoPlugin:pvgetTimeStamp";
    strcpy(timestamp, "timestamp in epics format");
    return true;
}

// caQtDM_Lib will call this routine for getting the timestamp for this monitor
int DemoPlugin::pvGetDescription(char *pv, char *description) {
    qDebug() << "DemoPlugin:pvGetDescription";
    strcpy(description, "hello, I am a double");
    return true;
}

// next routines are used to stop and restart the dataacquisition (used in case of tabWidgets in the display)
int DemoPlugin::pvClearEvent(void * ptr) {
    qDebug() << "DemoPlugin:pvClearEvent";
    return true;
}

int DemoPlugin::pvAddEvent(void * ptr) {
    qDebug() << "DemoPlugin:pvAddEvent";
    return true;
}

// next routines are used to Connect and disconnect monitors
int DemoPlugin::pvReconnect(knobData *kData) {
    qDebug() << "DemoPlugin:pvReconnect";
    return true;
}

int DemoPlugin::pvDisconnect(knobData *kData) {
    qDebug() << "DemoPlugin:pvDisconnect";
    return true;
} 

// flush any io
int DemoPlugin::FlushIO() {
    //qDebug() << "DemoPlugin:FlushIO";
    return true;
}

// termination
int DemoPlugin::TerminateIO() {
    //qDebug() << "DemoPlugin:TerminateIO";
    timerValues->stop();
    timer->stop();
    return true;
}

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
#else
    Q_EXPORT_PLUGIN2(DemoPlugin, DemoPlugin)
#endif

