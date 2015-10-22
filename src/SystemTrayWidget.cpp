// Windy - SystemTrayWidget.cpp
// Copyright (c) 2015 Arkadiusz Bokowy
//
// This file is a part of Windy.
//
// This projected is licensed under the terms of the MIT license.

#include "SystemTrayWidget.h"

#include "Settings.h"


SystemTrayWidget::SystemTrayWidget(QObject *parent) :
		QObject(parent),
		m_action_refresh(QIcon::fromTheme("view-refresh"), tr("&Refresh"), parent),
		m_action_preferences(QIcon::fromTheme("document-properties"), tr("&Preferences"), parent),
		m_action_about(QIcon::fromTheme("help-about"), tr("&About"), parent),
		m_action_quit(QIcon::fromTheme("application-exit"), tr("&Quit"), parent) {

	// assembly widget's context menu
	m_tray_icon.setContextMenu(&m_context_menu);
	m_context_menu.addAction(&m_action_refresh);
	m_context_menu.addAction(&m_action_preferences);
	m_context_menu.addAction(&m_action_about);
	m_context_menu.addSeparator();
	m_context_menu.addAction(&m_action_quit);

	connect(&m_tray_icon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
			this, SLOT(dispatchIconActivation(QSystemTrayIcon::ActivationReason)));

	connect(&m_action_refresh, SIGNAL(triggered()), SLOT(dispatchMenuAction()));
	connect(&m_action_preferences, SIGNAL(triggered()), SLOT(dispatchMenuAction()));
	connect(&m_action_about, SIGNAL(triggered()), SLOT(dispatchMenuAction()));
	connect(&m_action_quit, SIGNAL(triggered()), SLOT(dispatchMenuAction()));

	// initialize tray icon to the default value
	m_conditions.icon = WeatherConditions::WeatherIcon::Clear;
	updateIcon();

	m_tray_icon.show();

}

void SystemTrayWidget::setWeatherConditions(const WeatherConditions &conditions) {

	// update our internal weather conditions structure
	m_conditions = conditions;

	dumpWeatherConditions(conditions);

	updateIcon();
	updateToolTip();

}

void SystemTrayWidget::updateIcon() {

	QStringList icons;
	QString fallback;

	switch (m_conditions.icon) {
	case WeatherConditions::WeatherIcon::Clear:
		fallback = ":/icons/weather-clear";
		icons << "weather-clear-symbolic";
		icons << "weather-clear";
		break;
	case WeatherConditions::WeatherIcon::ClearNight:
		fallback = ":/icons/weather-clear-night";
		icons << "weather-clear-night-symbolic";
		icons << "weather-night-clear";
		break;
	case WeatherConditions::WeatherIcon::FewClouds:
		fallback = ":/icons/weather-few-clouds";
		icons << "weather-few-clouds-symbolic";
		icons << "weather-few-clouds";
		break;
	case WeatherConditions::WeatherIcon::FewCloudsNight:
		fallback = ":/icons/weather-few-clouds-night";
		icons << "weather-few-clouds-night-symbolic";
		icons << "weather-few-clouds-night";
		break;
	case WeatherConditions::WeatherIcon::Overcast:
		fallback = ":/icons/weather-overcast";
		icons << "weather-overcast-symbolic";
		icons << "weather-overcast";
		break;
	case WeatherConditions::WeatherIcon::Showers:
		fallback = ":/icons/weather-showers";
		icons << "weather-showers-symbolic";
		icons << "weather-showers";
		break;
	case WeatherConditions::WeatherIcon::ShowersScattered:
		fallback = ":/icons/weather-showers";
		icons << "weather-showers-scattered-symbolic";
		icons << "weather-showers-scattered";
		break;
	case WeatherConditions::WeatherIcon::Fog:
		fallback = ":/icons/weather-fog";
		icons << "weather-fog-symbolic";
		icons << "weather-fog";
		break;
	case WeatherConditions::WeatherIcon::Snow:
		fallback = ":/icons/weather-snow";
		icons << "weather-snow-symbolic";
		icons << "weather-snow";
		break;
	case WeatherConditions::WeatherIcon::Storm:
		fallback = ":/icons/weather-storm";
		icons << "weather-storm-symbolic";
		icons << "weather-storm";
		break;
	case WeatherConditions::WeatherIcon::SevereAlert:
		fallback = ":/icons/weather-severe-alert";
		icons << "weather-severe-alert-symbolic";
		icons << "weather-severe-alert";
		break;
	}

// By default, only X11 supports themed icons, and we are not going to
// install our theme on the user's operating system. However, we will
// ship bundled resources for systems other than Linux.
#ifdef Q_WS_X11

	// iterate over icon candidates and set the first one which is available
	for (QStringList::ConstIterator it = icons.begin(); it != icons.end(); it++) {
		QIcon icon(QIcon::fromTheme(*it));
		if (!icon.isNull()) {
			m_tray_icon.setIcon(icon);
			return;
		}
	}

	qWarning() << "Weather status icon not found:" << icons;

#endif // Q_WS_X11

	// NOTE: If themed icon was not found and windy was build without bundled
	//       icon pack, we might end up with a "blank" tray icon. Such a case
	//       should be treated as a misconfiguration, not a bug.
	m_tray_icon.setIcon(QIcon(fallback));
}

void SystemTrayWidget::updateToolTip() {

	Settings *settings = Settings::settings();

	QString unitTemperature;
	QString unitPressure;
	QString unitDistance;
	QString unitSpeed;

	QString temperature(tr("n/a"));
	QString pressure(tr("n/a"));
	QString windSpeed(tr("n/a"));
	QString windGustSpeed(tr("n/a"));
	QString windDirection(tr("n/a"));
	QString windChill(tr("n/a"));
	QString humidity(tr("n/a"));
	QString dewPoint(tr("n/a"));
	QString visibility(tr("n/a"));

	if (m_conditions.windDirection != -1)
		windDirection.setNum(m_conditions.windDirection, 'f', 0);
	if (m_conditions.humidity != -1)
		humidity.setNum(m_conditions.humidity, 'f', 0);

	switch (settings->getUnitTemperature()) {
	case Settings::UnitTemperature::Celsius:
		unitTemperature = trUtf8("°C");
		if (m_conditions.temperature != -1)
			temperature.setNum(m_conditions.temperature - 273.15, 'f', 1);
		if (m_conditions.windChill != -1)
			windChill.setNum(m_conditions.windChill - 273.15, 'f', 1);
		if (m_conditions.dewPoint != -1)
			dewPoint.setNum(m_conditions.dewPoint - 273.15, 'f', 1);
		break;
	case Settings::UnitTemperature::Fahrenheit:
		unitTemperature = trUtf8("°F");
		if (m_conditions.temperature != -1)
			temperature.setNum((m_conditions.temperature - 273.15) * 1.8 + 32, 'f', 1);
		if (m_conditions.windChill != -1)
			windChill.setNum((m_conditions.windChill - 273.15) * 1.8 + 32, 'f', 1);
		if (m_conditions.dewPoint != -1)
			dewPoint.setNum((m_conditions.dewPoint - 273.15) * 1.8 + 32, 'f', 1);
		break;
	case Settings::UnitTemperature::Kelvin:
		unitTemperature = trUtf8("K");
		if (m_conditions.temperature != -1)
			temperature.setNum(m_conditions.temperature, 'f', 1);
		if (m_conditions.windChill != -1)
			windChill.setNum(m_conditions.windChill, 'f', 1);
		if (m_conditions.dewPoint != -1)
			dewPoint.setNum(m_conditions.dewPoint, 'f', 1);
		break;
	}

	switch (settings->getUnitPressure()) {
	case Settings::UnitPressure::Hectopascal:
		unitPressure = trUtf8("hPa");
		if (m_conditions.pressure != -1)
			pressure.setNum(m_conditions.pressure / 100, 'f', 0);
		break;
	case Settings::UnitPressure::PoundPerSquareInch:
		unitPressure = trUtf8("psi");
		if (m_conditions.pressure != -1)
			pressure.setNum(m_conditions.pressure / 6894.75729, 'f', 1);
		break;
	case Settings::UnitPressure::MillimeterOfMercury:
		unitPressure = trUtf8("mmHg");
		if (m_conditions.pressure != -1)
			pressure.setNum(m_conditions.pressure / 133.3224, 'f', 0);
		break;
	}

	switch (settings->getUnitWindSpeed()) {
	case Settings::UnitWindSpeed::KilometerPerHour:
		unitDistance = trUtf8("km");
		unitSpeed = trUtf8("km/h");
		if (m_conditions.windSpeed != -1)
			windSpeed.setNum(m_conditions.windSpeed * 3.6, 'f', 0);
		if (m_conditions.windGustSpeed != -1)
			windGustSpeed.setNum(m_conditions.windGustSpeed * 3.6, 'f', 0);
		if (m_conditions.visibility != -1)
			visibility.setNum(m_conditions.visibility / 1000, 'f', 1);
		break;
	case Settings::UnitWindSpeed::MilePerHour:
		unitDistance = trUtf8("mi.");
		unitSpeed = trUtf8("MpH");
		if (m_conditions.windSpeed != -1)
			windSpeed.setNum(m_conditions.windSpeed / 0.44704, 'f', 0);
		if (m_conditions.windGustSpeed != -1)
			windGustSpeed.setNum(m_conditions.windGustSpeed / 0.44704, 'f', 0);
		if (m_conditions.visibility != -1)
			visibility.setNum(m_conditions.visibility / 1609.3472, 'f', 1);
		break;
	case Settings::UnitWindSpeed::MeterPerSecond:
		unitDistance = trUtf8("m");
		unitSpeed = trUtf8("m/s");
		if (m_conditions.windSpeed != -1)
			windSpeed.setNum(m_conditions.windSpeed, 'f', 1);
		if (m_conditions.windGustSpeed != -1)
			windGustSpeed.setNum(m_conditions.windGustSpeed, 'f', 1);
		if (m_conditions.visibility != -1)
			visibility.setNum(m_conditions.visibility, 'f', 0);
		break;
	}

	// trim location name if it exceeds 30 characters (prevent wrapping)
	QString locationName(m_conditions.locationName);
	if (locationName.length() > 30) {

		// strip all non-alphanumerical characters from the trimmed end
		auto i = locationName.constBegin() + 28;
		for (; i != locationName.constBegin() - 1; i--)
			if ((*i).isLetterOrNumber())
				break;

		locationName = locationName.left(i - locationName.constBegin() + 1);
		if (locationName.isEmpty())
			// it has turned out that we've exterminated all characters from
			// the location name string, so restore the original text
			locationName = m_conditions.locationName;
		else
			locationName += "...";

	}

	QString messageTemplate(tr("<b>{NAME}</b><br/>"));
	if (settings->getShowTemperature()) {
		if (settings->getShowWindChill())
			messageTemplate += tr("Temperature: {TEMP} ({CHILL}) {T}<br/>");
		else
			messageTemplate += tr("Temperature: {TEMP} {T}<br/>");
	}
	if (settings->getShowWindSpeed()) {
		QString windTemplate;
		if (settings->getShowWindGustSpeed())
			windTemplate = tr("Wind: {WIND} ({GUST}) {S}");
		else
			windTemplate = tr("Wind: {WIND} {S}");
		if (m_conditions.windDirection != -1)
			windTemplate += trUtf8(" {WDIR}°");
		messageTemplate += windTemplate + "<br/>";
	}
	if (settings->getShowVisibility())
		messageTemplate += tr("Visibility: {VISIB} {D}<br/>");
	if (settings->getShowPressure())
		messageTemplate += tr("Pressure: {PRES} {P}<br/>");

	m_tray_icon.setToolTip(messageTemplate
			.replace("{T}", unitTemperature)
			.replace("{P}", unitPressure)
			.replace("{D}", unitDistance)
			.replace("{S}", unitSpeed)
			.replace("{NAME}", locationName)
			.replace("{TEMP}", temperature)
			.replace("{PRES}", pressure)
			.replace("{WIND}", windSpeed)
			.replace("{GUST}", windGustSpeed)
			.replace("{WDIR}", windDirection)
			.replace("{CHILL}", windChill)
			.replace("{HUMID}", humidity)
			.replace("{DEWPT}", dewPoint)
			.replace("{VISIB}", visibility));

}

void SystemTrayWidget::dispatchIconActivation(QSystemTrayIcon::ActivationReason reason) {
	// NOTE: Icon activation dispatcher handles left and middle click only. Other
	//       actions are handled internally by this widget, so there is no reason
	//       for them to be exposed.
	if (reason == QSystemTrayIcon::Trigger || reason == QSystemTrayIcon::MiddleClick)
		emit iconActivated(reason);
}

void SystemTrayWidget::dispatchMenuAction() {

	QAction *action = qobject_cast<QAction *>(sender());

	if (action == &m_action_refresh)
		emit menuActionTriggered(MenuAction::Refresh);
	else if (action == &m_action_preferences)
		emit menuActionTriggered(MenuAction::Preferences);
	else if (action == &m_action_about)
		emit menuActionTriggered(MenuAction::About);
	else if (action == &m_action_quit)
		emit menuActionTriggered(MenuAction::Quit);

}
