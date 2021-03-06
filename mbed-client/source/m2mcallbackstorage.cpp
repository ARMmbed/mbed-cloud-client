/*
 * Copyright (c) 2017 ARM Limited. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "include/m2mcallbackstorage.h"

#include <stddef.h>


// Dummy constructor, which does not init any value to something meaningful but needed for array construction.
// It is better to leave values unintialized, so the Valgrind will point out if the Vector is used without
// setting real values in there.
M2MCallbackAssociation::M2MCallbackAssociation()
{
}

M2MCallbackAssociation::M2MCallbackAssociation(const M2MBase *object, void *callback, M2MCallbackType type, void *client_args)
: _object(object), _callback(callback), _type(type), _client_args(client_args)
{
}

M2MCallbackStorage* M2MCallbackStorage::_static_instance = NULL;

M2MCallbackStorage *M2MCallbackStorage::get_instance()
{
    if (M2MCallbackStorage::_static_instance == NULL) {
        M2MCallbackStorage::_static_instance = new M2MCallbackStorage();
    }
    return M2MCallbackStorage::_static_instance;
}

void M2MCallbackStorage::delete_instance()
{
    delete M2MCallbackStorage::_static_instance;
    M2MCallbackStorage::_static_instance = NULL;
}

M2MCallbackStorage::~M2MCallbackStorage()
{
    // Go through the list and delete all the FP<n> objects if there are any.
    // If the system is done properly, each m2mobject should actually
    // remove its callbacks from its destructor so there is nothing here to do
    // find any association to given object and delete them from the vector
    for (int index = _callbacks.size()-1; index >=0; index --) {
        _callbacks.erase(index);
    }
}

bool M2MCallbackStorage::add_callback(const M2MBase &object,
                                      void *callback,
                                      M2MCallbackAssociation::M2MCallbackType type,
                                      void *client_args)
{
    bool add_success = false;
    M2MCallbackStorage* instance = get_instance();
    if (instance) {

        add_success = instance->do_add_callback(object, callback, type, client_args);
    }
    return add_success;
}

bool M2MCallbackStorage::do_add_callback(const M2MBase &object, void *callback, M2MCallbackAssociation::M2MCallbackType type, void *client_args)
{
    bool add_success = false;

    // verify that the same callback is not re-added.
    if (!does_callback_exist(object, callback, type)) {

        const M2MCallbackAssociation association(&object, callback, type, client_args);
        _callbacks.push_back(association);
        add_success = true;
    }
    return add_success;
}

#if 0 // Functions not used currently
void M2MCallbackStorage::remove_callbacks(const M2MBase &object)
{
   // do not use the get_instance() here as it might create the instance
    M2MCallbackStorage* instance = M2MCallbackStorage::_static_instance;
    if (instance) {

        instance->do_remove_callbacks(object);
    }
}

void M2MCallbackStorage::do_remove_callbacks(const M2MBase &object)
{
    // find any association to given object and delete them from the vector
    for (int index = 0; index < _callbacks.size();) {
        if (_callbacks[index]._object == &object) {
            _callbacks.erase(index);
        } else {
            index++;
        }
    }
}
#endif

void* M2MCallbackStorage::remove_callback(const M2MBase &object, M2MCallbackAssociation::M2MCallbackType type)
{
    void* callback = NULL;
    // do not use the get_instance() here as it might create the instance
    M2MCallbackStorage* instance = M2MCallbackStorage::_static_instance;
    if (instance) {
        callback = instance->do_remove_callback(object, type);
    }
    return callback;
}

void* M2MCallbackStorage::do_remove_callback(const M2MBase &object, M2MCallbackAssociation::M2MCallbackType type)
{
    void* callback = NULL;
    for (int index = 0; index < _callbacks.size(); index++) {

        if ((_callbacks[index]._object == &object) && (_callbacks[index]._type == type)) {
            callback = _callbacks[index]._callback;
            _callbacks.erase(index);
        }
    }
    return callback;
}

void* M2MCallbackStorage::get_callback(const M2MBase &object, M2MCallbackAssociation::M2MCallbackType type)
{
    void* callback = NULL;
    const M2MCallbackStorage* instance = get_instance();
    if (instance) {

        callback = instance->do_get_callback(object, type);
    }
    return callback;
}

void* M2MCallbackStorage::do_get_callback(const M2MBase &object, M2MCallbackAssociation::M2MCallbackType type) const
{
    void* callback = NULL;
    if (!_callbacks.empty()) {
        M2MCallbackAssociationList::const_iterator it = _callbacks.begin();

        for ( ; it != _callbacks.end(); it++ ) {

            if ((it->_object == &object) && (it->_type == type)) {
                callback = it->_callback;
                break;
            }
        }
    }
    return callback;
}

M2MCallbackAssociation* M2MCallbackStorage::get_association_item(const M2MBase &object, M2MCallbackAssociation::M2MCallbackType type)
{
    M2MCallbackAssociation* callback_association = NULL;
    const M2MCallbackStorage* instance = get_instance();
    if (instance) {

        callback_association = instance->do_get_association_item(object, type);
    }
    return callback_association;
}

M2MCallbackAssociation* M2MCallbackStorage::do_get_association_item(const M2MBase &object, M2MCallbackAssociation::M2MCallbackType type) const
{
    M2MCallbackAssociation* callback_association = NULL;
    if (!_callbacks.empty()) {
        M2MCallbackAssociationList::const_iterator it = _callbacks.begin();

        for ( ; it != _callbacks.end(); it++ ) {

            if ((it->_object == &object) && (it->_type == type)) {
                callback_association = (M2MCallbackAssociation*)it;
                break;
            }
        }
    }
    return callback_association;
}

bool M2MCallbackStorage::does_callback_exist(const M2MBase &object, M2MCallbackAssociation::M2MCallbackType type)
{
    bool found = false;
    if (get_callback(object, type) != NULL) {
        found = true;
    }
    return found;
}

bool M2MCallbackStorage::does_callback_exist(const M2MBase &object, void *callback, M2MCallbackAssociation::M2MCallbackType type) const
{
    bool match_found = false;

    if (!_callbacks.empty()) {
        M2MCallbackAssociationList::const_iterator it = _callbacks.begin();

        for ( ; it != _callbacks.end(); it++ ) {

            if ((it->_object == &object) && (it->_callback == callback) && (it->_type == type)) {
                match_found = true;
                break;
            }
        }
    }

    return match_found;
}
