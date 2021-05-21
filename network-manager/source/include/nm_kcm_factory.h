/*
 * Copyright (c) 2020-2021 Pelion. All rights reserved.
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

#ifndef NM_KCM_FACTORY_H_
#define NM_KCM_FACTORY_H_
#include "ns_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * \brief Initialize Wi-SUN network name
 *
 * \param network_name_buf Address for network name buffer
 * \return NM_STATUS_SUCCESS if network name is read from kcm, caller must free the memory.
 * \return NM_STATUS_FAIL in case of configuration is not available.
 *
 */
nm_status_t nm_kcm_wisun_network_name_init(char **network_name_buf);

/*
 * \brief Initialize Wi-SUN network size
 *
 * \param network_size Address to write network size
 * \return NM_STATUS_SUCCESS on success.
 * \return NM_STATUS_FAIL in case of configuration is not available.
 *
 */
nm_status_t nm_kcm_wisun_network_size_init(uint8_t *network_size);

/*
 * \brief Initialize Wi-SUN mesh MAC address.
 *
 * \param mesh_mac_address_ptr Pointer where address can be read
 * \param mesh_mac_address_len Length of data
 * \return NM_STATUS_SUCCESS if configuration is read successfully, caller must free the memory.
 * \return NM_STATUS_FAIL in case of configuration is not available.
 *
 */
nm_status_t nm_kcm_mesh_mac_address_init(uint8_t **mesh_mac_address_ptr, size_t *mesh_mac_address_len);

/*
 * \brief Initialize Wi-SUN Backhaul MAC address.
 *
 * \param eth_mac_address_ptr Pointer where address can be read
 * \param eth_mac_address_len Length of data
 * \return NM_STATUS_SUCCESS if configuration is read successfully, caller must free the memory.
 * \return NM_STATUS_FAIL in case of configuration is not available.
 *
 */
nm_status_t nm_kcm_ethernet_mac_address_init(uint8_t **eth_mac_address_ptr, size_t *eth_mac_address_len);

/*
 * \brief Initialize Wi-SUN network regulatory domain, operating mode and class.
 *
 * \param regulatory_domain address to write regulatory_domain
 * \param operating_class address to write operating_class
 * \param operating_mode address to write operating_mode
 * \return NM_STATUS_SUCCESS on success.
 * \return NM_STATUS_FAIL if regulatory domain, operating class and operating mode is not available.
 */
nm_status_t nm_kcm_wisun_network_regulatory_domain_init(uint8_t *regulatory_domain, uint8_t *operating_class, uint8_t *operating_mode);

/*
 * \brief Initialize Wi-SUN network radius server address.
 *
 * \param srv_addr Pointer where address can be read
 * \param srv_addr_len Length of data
 * \return NM_STATUS_SUCCESS if configuration is read successfully, caller must free the memory.
 * \return NM_STATUS_FAIL in case of configuration is not available.
 *
 */
nm_status_t nm_kcm_wisun_network_radius_addr_init(char **srv_addr, size_t *srv_addr_len);

/*
 * \brief Initialize Wi-SUN network radius server secret.
 *
 * Shared secret can be ascii string (not NULL terminated) or byte array.
 *
 * \param srv_secret_buf Pointer where secret data will be available.
 * \param actual_secret_len Actual secret length in successful case.
 * \return NM_STATUS_SUCCESS if configuration read successfully, caller must free the memory.
 * \return NM_STATUS_FAIL in case of configuration is not available
 *
 */
nm_status_t nm_kcm_wisun_network_radius_secret_init(uint8_t **srv_secret_buf, size_t *actual_secret_len);

/*
 * \brief Initialize Wi-SUN network trusted certificate.
 *
 * Trusted CA certificates are all trusted roots and intermediate certificates concatenated into one string.
 *
 * \param trusted_cert_buf Pointer where trusted certificates will be available.
 * \param trusted_cert_len Actual trusted certificate length in successful case.
 * \return NM_STATUS_SUCCESS if configuration read successfully.
 * \return NM_STATUS_FAIL in case of configuration is not available
 *
 */
nm_status_t nm_kcm_wisun_network_trusted_certificate_init(uint8_t **trusted_cert_buf, uint16_t *trusted_cert_len);

/*
 * \brief Initialize Wi-SUN network own certificate and private kay.
 *
 * Own certificate is either one or multiple certificate chain concatenated into one string.
 *
 * \param own_cert_buf Pointer where own certificates will be available.
 * \param own_cert_len Own certificate length in successful case.
 * \param own_cert_key Pointer where own certificate keys will be available.
 * \param own_cert_key_len Own certificate key length in successful case.
 * \return NM_STATUS_SUCCESS if configuration read successfully.
 * \return NM_STATUS_FAIL in case of configuration is not available
 *
 */
nm_status_t nm_kcm_wisun_network_own_certificate_init(uint8_t **own_cert_buf, uint16_t *own_cert_len, uint8_t **own_cert_key, uint16_t *own_cert_key_len);


#ifdef __cplusplus
}
#endif
#endif /* NM_KCM_FACTORY_H_ */
