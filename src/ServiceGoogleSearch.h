// Windy - ServiceGoogleSearch.h
// Copyright (c) 2015 Arkadiusz Bokowy
//
// This file is a part of Windy.
//
// This projected is licensed under the terms of the MIT license.

#ifndef __SERVICE_GOOGLE_SEARCH_H
#define __SERVICE_GOOGLE_SEARCH_H

#include "WeatherService.h"

#include <QNetworkAccessManager>


class ServiceGoogleSearch : public WeatherService {
	Q_OBJECT

public:
	explicit ServiceGoogleSearch(QObject *parent = 0);

public slots:
	virtual bool fetchCurrentConditions();
	virtual bool fetchForecastConditions() { return false; }
	virtual bool fetchLocationAutocomplete(const QString &query) { return false; }

private slots:
	void dispatchCurrentConditions();

protected:
	QNetworkRequest buildNetworkRequest(const QUrl &url);

private:
	QNetworkAccessManager m_network_manager;

};

#endif