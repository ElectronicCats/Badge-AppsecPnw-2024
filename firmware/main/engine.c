#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "engine.h"

OWASPVulns vulns[] = {
    {"A01", "Broken Access Control"},
    {"A02", "Cryptographic Failures"},
    {"A03", "Injection"},
    {"A04", "Insecure Design"},
    {"A05", "Security Misconfiguration"},
    {"A06", "Vulnerable outdated Components"},
    {"A07", "Identification Authentication Failures"},
    {"A08", "Software and Data Integrity Failures"},
    {"A09", "Security Logging and Monitoring Failures"},
    {"A10", "Server Side Request Forgery"}
};

OWASPBaseAction baseActions[] = {
{"CWE-25", "Path Traversal"},
{"CWE-25R", "Validate Input"},
{"CWE-352", "Cross-Site Request Forgery"},
{"CWE-352R", "Anti-CSRF"},
{"CWE-261", "Weak Encoding Password"},
{"CWE-261R", "128 bits Encryption"},
{"CWE-319", "Cleartext Transmission"},
{"CWE-319R", "SSL/Cryptographic"},
{"CWE-20", "Improper Input Validation"},
{"CWE-20R", "Client/Server Validation"},
{"CWE-89", "SQL Injection"},
{"CWE-89R", "Isolated account"},
{"CWE-256", "Unprotected Storage of Credentials"},
{"CWE-256R", "Hashes of password"},
{"CWE-434", "Unrestricted Upload"},
{"CWE-434R", "Generate Hashes of Files"},
{"CWE-260", "Password in Configuration File"},
{"CWE-260R", "Static Application Security Testing"},
{"CWE-315", "Cleartext storage"},
{"CWE-315R", "Encrypt Information"},
{"CWE-937", "Using Commponents with Known Vulns"},
{"CWE-937R", "Update Version"},
{"CWE-319", "Use of Unmaintained Commponents"},
{"CWE-319R", "Change Component"},
{"CWE-259", "Use of Hardcoded Credentials"},
{"CWE-259R", "Store password outside of code"},
{"CWE-307", "Improper Handler Authentication Attempts"},
{"CWE-307R", "Implemment Lockout Mechanism"},
{"CWE-426", "Untrusted Search Path"},
{"CWE-426R", "Use function that require explicit path"},
{"CWE-494", "Download of Code Without Integrity Check"},
{"CWE-494R", "Implement Integrity Check"},
{"CWE-532", "Insertion of Sensitive Information"},
{"CWE-532R", "Protect Log files from unauthorized access"},
{"CWE-778", "Insufficient Logging"},
{"CWE-778R", "Centralized logging mmechanism"},
{"CWE-918", "Server-Side Request Forgery"},
{"CWE-918R", "Restrict access to internal network"}
};

OWASPAction actions[] = {
{ &baseActions[0], &baseActions[1] }, //CWE-25, CWE-25R
{ &baseActions[2], &baseActions[3] }, //CWE-352, CWE-352R
{ &baseActions[4], &baseActions[5] }, //CWE-261, CWE-261R
{ &baseActions[6], &baseActions[7] }, //CWE-319, CWE-319R
{ &baseActions[8], &baseActions[9] }, //CWE-20, CWE-20R
{ &baseActions[10], &baseActions[11] }, //CWE-89, CWE-89R
{ &baseActions[12], &baseActions[13] }, //CWE-256, CWE-256R
{ &baseActions[14], &baseActions[15] }, //CWE-434, CWE-434R
{ &baseActions[16], &baseActions[17] }, //CWE-260, CWE-260R
{ &baseActions[18], &baseActions[19] }, //CWE-315, CWE-315R
{ &baseActions[20], &baseActions[21] }, //CWE-937, CWE-937R
{ &baseActions[22], &baseActions[23] }, //CWE-319, CWE-319R
{ &baseActions[24], &baseActions[25] }, //CWE-259, CWE-259R
{ &baseActions[26], &baseActions[27] }, //CWE-307, CWE-307R
{ &baseActions[28], &baseActions[29] }, //CWE-426, CWE-426R
{ &baseActions[30], &baseActions[31] }, //CWE-494, CWE-494R
{ &baseActions[32], &baseActions[33] }, //CWE-532, CWE-532R
{ &baseActions[34], &baseActions[35] }, //CWE-778, CWE-778R
{ &baseActions[36], &baseActions[37] }, //CWE-918, CWE-918R
};

OWASPProfile profiles[] = {
{ &vulns[0], &actions[0], &actions[1] }, // A01
{ &vulns[1], &actions[2], &actions[3] }, // A02
{ &vulns[2], &actions[4], &actions[5] }, // A03
{ &vulns[3], &actions[6], &actions[7] }, // A04
{ &vulns[4], &actions[8], &actions[9] }, // A05
{ &vulns[5], &actions[10], &actions[11] }, // A06
{ &vulns[6], &actions[12], &actions[13] }, // A07
{ &vulns[7], &actions[14], &actions[15] }, // A08
{ &vulns[8], &actions[16], &actions[17] }, // A09
{ &vulns[9], &actions[18], &actions[19] } // A10
};


/**
 * @brief Convert the OWASP profile struct to a uint8_t array.
 *
 * @param dest_buffer The pointer to the destination buffer.
 * @param profile The OWASP profile pointer to convert.
 * @return The CWE value.
 */
void get_owasp_profile_cwe_uint8(uint8_t* dest_buffer, OWASPProfile *profile) {
    for (int i = 0; i < sizeof(profile->vuln->cwe); i++) {
        dest_buffer[i] = profile->vuln->cwe[i];
    }
}

void create_owasp_commmand(uint8_t* dest_buffer, OWASPProfile *profile) {
    dest_buffer[0] = '1'; // Agregar el número 1 después del primer carácter
    dest_buffer[1] = profile->vuln->cwe[0]; // Copiar el primer carácter (el número)
    for (int i = 1; i < sizeof(profile->vuln->cwe); i++) {
        dest_buffer[i+1] = profile->vuln->cwe[i]; // Copiar el resto de los caracteres
    }
    // dest_buffer[0] = '1';
    // dest_buffer[1] = profile->vuln->cwe[0];
    // dest_buffer[2] = profile->vuln->cwe[1];
    // dest_buffer[3] = profile->vuln->cwe[2];
    
}
/**
 * @brief Get the OWASP profile by index.
 *
 * @param index The index of the OWASP profile.
 * @return The OWASP profile pointer.
 */
OWASPProfile* getOWASPProfile(int index) {
    if (index >= 0 && index < sizeof(profiles) / sizeof(profiles[0])) {
        return &profiles[index];
    }
    return &profiles[0];
}


