/******************************************************************************
 * Icinga 2                                                                   *
 * Copyright (C) 2012 Icinga Development Team (http://www.icinga.org/)        *
 *                                                                            *
 * This program is free software; you can redistribute it and/or              *
 * modify it under the terms of the GNU General Public License                *
 * as published by the Free Software Foundation; either version 2             *
 * of the License, or (at your option) any later version.                     *
 *                                                                            *
 * This program is distributed in the hope that it will be useful,            *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of             *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the              *
 * GNU General Public License for more details.                               *
 *                                                                            *
 * You should have received a copy of the GNU General Public License          *
 * along with this program; if not, write to the Free Software Foundation     *
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.             *
 ******************************************************************************/

#include "livestatus/contactstable.h"
#include "icinga/user.h"
#include "icinga/timeperiod.h"
#include "base/dynamictype.h"
#include "base/objectlock.h"
#include <boost/smart_ptr/make_shared.hpp>
#include <boost/foreach.hpp>
#include <boost/tuple/tuple.hpp>

using namespace icinga;
using namespace livestatus;

ContactsTable::ContactsTable(void)
{
	AddColumns(this);
}

void ContactsTable::AddColumns(Table *table, const String& prefix,
	const Column::ObjectAccessor& objectAccessor)
{
	table->AddColumn(prefix + "name", Column(&ContactsTable::NameAccessor, objectAccessor));
	table->AddColumn(prefix + "alias", Column(&ContactsTable::NameAccessor, objectAccessor));
	table->AddColumn(prefix + "email", Column(&ContactsTable::EmailAccessor, objectAccessor));
	table->AddColumn(prefix + "pager", Column(&ContactsTable::PagerAccessor, objectAccessor));
	table->AddColumn(prefix + "host_notification_period", Column(&ContactsTable::HostNotificationPeriodAccessor, objectAccessor));
	table->AddColumn(prefix + "service_notification_period", Column(&ContactsTable::ServiceNotificationPeriodAccessor, objectAccessor));
	table->AddColumn(prefix + "can_submit_commands", Column(&ContactsTable::CanSubmitCommandsAccessor, objectAccessor));
	table->AddColumn(prefix + "host_notifications_enabled", Column(&ContactsTable::HostNotificationsEnabledAccessor, objectAccessor));
	table->AddColumn(prefix + "service_notifications_enabled", Column(&ContactsTable::ServiceNotificationsEnabledAccessor, objectAccessor));
	table->AddColumn(prefix + "in_host_notification_period", Column(&ContactsTable::InHostNotificationPeriodAccessor, objectAccessor));
	table->AddColumn(prefix + "in_service_notification_period", Column(&ContactsTable::InServiceNotificationPeriodAccessor, objectAccessor));
	table->AddColumn(prefix + "custom_variable_names", Column(&ContactsTable::CustomVariableNamesAccessor, objectAccessor));
	table->AddColumn(prefix + "custom_variable_values", Column(&ContactsTable::CustomVariableValuesAccessor, objectAccessor));
	table->AddColumn(prefix + "custom_variables", Column(&ContactsTable::CustomVariablesAccessor, objectAccessor));
	table->AddColumn(prefix + "modified_attributes", Column(&ContactsTable::ModifiedAttributesAccessor, objectAccessor));
	table->AddColumn(prefix + "modified_attributes_list", Column(&ContactsTable::ModifiedAttributesListAccessor, objectAccessor));
}

String ContactsTable::GetName(void) const
{
	return "contacts";
}

void ContactsTable::FetchRows(const AddRowFunction& addRowFn)
{
	BOOST_FOREACH(const DynamicObject::Ptr& object, DynamicType::GetObjects("User")) {
		addRowFn(object);
	}
}

Value ContactsTable::NameAccessor(const Value& row)
{
	return static_cast<User::Ptr>(row)->GetName();
}

Value ContactsTable::AliasAccessor(const Value& row)
{
	return static_cast<User::Ptr>(row)->GetDisplayName();
}

Value ContactsTable::EmailAccessor(const Value& row)
{
	Dictionary::Ptr macros = static_cast<User::Ptr>(row)->GetMacros();

	if (!macros)
		return Empty;

	return macros->Get("email");
}

Value ContactsTable::PagerAccessor(const Value& row)
{
	Dictionary::Ptr macros = static_cast<User::Ptr>(row)->GetMacros();

	if (!macros)
		return Empty;

	return macros->Get("pager");
}

Value ContactsTable::HostNotificationPeriodAccessor(const Value& row)
{
	/* same as service */
	TimePeriod::Ptr timeperiod = static_cast<User::Ptr>(row)->GetNotificationPeriod();

	if (!timeperiod)
		return Empty;

	return timeperiod->GetName();
}

Value ContactsTable::ServiceNotificationPeriodAccessor(const Value& row)
{
	TimePeriod::Ptr timeperiod = static_cast<User::Ptr>(row)->GetNotificationPeriod();

	if (!timeperiod)
		return Empty;

	return timeperiod->GetName();
}

Value ContactsTable::CanSubmitCommandsAccessor(const Value& row)
{
	/* default enabled */
	return 1;
}

Value ContactsTable::HostNotificationsEnabledAccessor(const Value& row)
{
	return (static_cast<User::Ptr>(row)->GetEnableNotifications() ? 1 : 0);
}

Value ContactsTable::ServiceNotificationsEnabledAccessor(const Value& row)
{
	return (static_cast<User::Ptr>(row)->GetEnableNotifications() ? 1 : 0);
}

Value ContactsTable::InHostNotificationPeriodAccessor(const Value& row)
{
	TimePeriod::Ptr timeperiod = static_cast<User::Ptr>(row)->GetNotificationPeriod();

	if (!timeperiod)
		return Empty;

	return (timeperiod->IsInside(Utility::GetTime()) ? 1 : 0);
}

Value ContactsTable::InServiceNotificationPeriodAccessor(const Value& row)
{
	TimePeriod::Ptr timeperiod = static_cast<User::Ptr>(row)->GetNotificationPeriod();

	if (!timeperiod)
		return Empty;

	return (timeperiod->IsInside(Utility::GetTime()) ? 1 : 0);
}

Value ContactsTable::CustomVariableNamesAccessor(const Value& row)
{
	Dictionary::Ptr custom = static_cast<User::Ptr>(row)->GetCustom();

	if (!custom)
		return Empty;

	Array::Ptr cv = boost::make_shared<Array>();

	ObjectLock olock(custom);
	String key;
	Value value;
	BOOST_FOREACH(boost::tie(key, value), custom) {
		if (key == "notes" ||
		    key == "action_url" ||
		    key == "notes_url" ||
		    key == "icon_image" ||
		    key == "icon_image_alt" ||
		    key == "statusmap_image" ||
		    key == "2d_coords")
			continue;

		cv->Add(key);
	}

	return cv;
}

Value ContactsTable::CustomVariableValuesAccessor(const Value& row)
{
	Dictionary::Ptr custom = static_cast<User::Ptr>(row)->GetCustom();

	if (!custom)
		return Empty;

	Array::Ptr cv = boost::make_shared<Array>();

	ObjectLock olock(custom);
	String key;
	Value value;
	BOOST_FOREACH(boost::tie(key, value), custom) {
		if (key == "notes" ||
		    key == "action_url" ||
		    key == "notes_url" ||
		    key == "icon_image" ||
		    key == "icon_image_alt" ||
		    key == "statusmap_image" ||
		    key == "2d_coords")
			continue;

		cv->Add(value);
	}

	return cv;
}

Value ContactsTable::CustomVariablesAccessor(const Value& row)
{
	Dictionary::Ptr custom = static_cast<User::Ptr>(row)->GetCustom();

	if (!custom)
		return Empty;

	Array::Ptr cv = boost::make_shared<Array>();

	ObjectLock olock(custom);
	String key;
	Value value;
	BOOST_FOREACH(boost::tie(key, value), custom) {
		if (key == "notes" ||
		    key == "action_url" ||
		    key == "notes_url" ||
		    key == "icon_image" ||
		    key == "icon_image_alt" ||
		    key == "statusmap_image" ||
		    key == "2d_coords")
			continue;

		Array::Ptr key_val = boost::make_shared<Array>();
		key_val->Add(key);
		key_val->Add(value);
		cv->Add(key_val);
	}

	return cv;
}

Value ContactsTable::ModifiedAttributesAccessor(const Value& row)
{
	/* not supported */
	return Empty;
}

Value ContactsTable::ModifiedAttributesListAccessor(const Value& row)
{
	/* not supported */
	return Empty;
}
