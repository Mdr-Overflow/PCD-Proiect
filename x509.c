// #include <bits/getopt_core.h>

#include <stdio.h>
#include <stdlib.h>

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/x509.h>
#include <stdbool.h>
#include <string.h>

#pragma GCC diagnostic ignored "-Wdeprecated-declarations"




void Sanitise(char *str) {
    
    char *src = str, *dst = str;
    
    while (*src != '\0') {
        if (*src == ' ' || *src == '\n' || *src == '=') {
            src++;
        } else {
            *dst++ = *src++;
        }
    }
    *dst = '\0';
}


EVP_PKEY* generate_key() {
 
  EVP_PKEY* pkey = EVP_PKEY_new();

  /* Generate a RSA key and assign it to pkey.
   * RSA_generate_key is deprecated.
   */
  BIGNUM* bne = BN_new();
  BN_set_word(bne, RSA_F4);
  RSA* rsa = RSA_new();
  RSA_generate_key_ex(rsa, 2048, bne, NULL);

  EVP_PKEY_assign_RSA(pkey, rsa);

  return pkey;
}



X509* generate_x509 (EVP_PKEY* pkey, char * validity_period , char * name , char * issuer, int SN, 
 char* org, char* org_unit, char* locality, char* state, char* country) {
  X509* x509 = X509_new();

  /* set a few parameters of the certificate. */

  /* certificate expiration date: 365 days from now (60s * 60m * 24h * 365d) */
  int days = atoi(validity_period);
  X509_gmtime_adj(X509_get_notBefore(x509), 0);
  X509_gmtime_adj(X509_get_notAfter(x509), days * 24 * 60 * 60); // zile * ore * minute * secunde

  X509_set_pubkey(x509, pkey);


//   X509_set_issuer_name(x509, issuer);

  

    // // Setarea versiunii certificatului la v3

     X509_set_version(x509, 3);

     // Setarea numărului serial unic pentru certificat
     ASN1_INTEGER_set(X509_get_serialNumber(x509), SN);

    X509_NAME *name_obj = X509_NAME_new();

    //Adding organization name
    if (org != NULL) {
        X509_NAME_add_entry_by_txt(name_obj, "C", MBSTRING_ASC, (unsigned char*)country, -1, -1, 0);
    }

    // Adding organizational unit name
    if (org_unit != NULL) {
        X509_NAME_add_entry_by_txt(name_obj, "ST", MBSTRING_ASC, (unsigned char*)state, -1, -1, 0);
    }

    // Adding locality name
    if (locality != NULL) {
        X509_NAME_add_entry_by_txt(name_obj, "L", MBSTRING_ASC, (unsigned char*)locality, -1, -1, 0);
    }

    // Adding state name
    if (state != NULL) {
        X509_NAME_add_entry_by_txt(name_obj, "O", MBSTRING_ASC, (unsigned char*)org, -1, -1, 0);
    }

    // Adding country name
    if (country != NULL) {
        X509_NAME_add_entry_by_txt(name_obj, "OU", MBSTRING_ASC, (unsigned char*)org_unit, -1, -1, 0);
    }

     // Setarea numelui emițătorului (issuer) și al subiectului
   
    
    X509_NAME_add_entry_by_txt(name_obj, "CN", MBSTRING_ASC, (unsigned char*)issuer, -1, -1, 0);
    X509_set_issuer_name(x509, name_obj);
    X509_NAME_free(name_obj);

    name_obj = X509_NAME_new();
    X509_NAME_add_entry_by_txt(name_obj, "CN", MBSTRING_ASC, (unsigned char*)name, -1, -1, 0);
    X509_set_subject_name(x509, name_obj);
    X509_NAME_free(name_obj);


  /* finally sign the certificate with the key. */
  X509_sign(x509, pkey, EVP_sha256());
 // free(name_obj);
  return x509;
}





int isCertificateValid(X509* cert, X509_CRL *crl) {
    time_t now = time(NULL);
    struct tm* current_time = localtime(&now);

    // Check if the current time is within the certificate's validity period
    int valid = X509_cmp_current_time(X509_get_notBefore(cert));
    if (valid == 0) {
        printf("-- The certificate is not yet valid -- \n");
        return 0;
    } else if (valid > 0) {
        printf(" --The certificate has expired -- \n");
        return 0;
    }

    valid = X509_cmp_current_time(X509_get_notAfter(cert));
    if (valid == 0) {
        printf(" --The certificate is not yet valid-- \n");
        return 0;
    } else if (valid < 0) {
        printf(" --The certificate has expired -- \n");
        return 0;
    }


// Check revocation list if available
  if (crl != NULL) {
    STACK_OF(X509_REVOKED)* revoked_stack = X509_CRL_get_REVOKED(crl);
    if (revoked_stack != NULL) {
        int num_revoked = sk_X509_REVOKED_num(revoked_stack);
        X509_REVOKED* revoked;
        for (int i = 0; i < num_revoked; i++) {
            revoked = sk_X509_REVOKED_value(revoked_stack, i);
                printf("\n --The certificate has been REVOKED--.\n");
                
              //  X509_REVOKED_free(revoked);
            //    sk_X509_REVOKED_free(revoked_stack);
                
                return 0;
        }
      //    X509_REVOKED_free(revoked);
    
    }
 //   sk_X509_REVOKED_free(revoked_stack);
  }

    
    printf("\n --cert is valid-- \n");
    return 1;
}


X509_CRL* create_crl() {
  X509_CRL* crl = X509_CRL_new();
  return crl;
}


bool check_certificate_valid_LEGACY(X509* x509) {
  X509_STORE_CTX* ctx = X509_STORE_CTX_new();
  X509_STORE* store = X509_STORE_new();

  X509_STORE_add_cert(store, x509);
  X509_STORE_CTX_init(ctx, store, x509, NULL);

  return X509_verify_cert(ctx) == 1? true : false;
}



void Revoc_cert( X509_CRL * crl , X509* cert, EVP_PKEY* issuer_key, int serial, char* reason) {
  X509_REVOKED* revoked = X509_REVOKED_new();
  ASN1_INTEGER_set((ASN1_INTEGER *) X509_REVOKED_get0_serialNumber(revoked), serial);
  X509_gmtime_adj((ASN1_TIME*) X509_REVOKED_get0_revocationDate(revoked), 0);
  X509_REVOKED_set_revocationDate(revoked,(ASN1_TIME*) X509_REVOKED_get0_revocationDate(revoked));
  X509_CRL_add0_revoked(crl, revoked);
  X509_CRL_sort(crl);
  X509_CRL_sign(crl, issuer_key, EVP_sha256());

}


X509* Extend_CERT (EVP_PKEY* pkey, char * validity_period , char * name , char * issuer, int SN, 
 char* org, char* org_unit, char* locality, char* state, char* country, char* cert_path, char* key_path) {
  
  X509* x509 = X509_new();

  /* load existing certificate and key */
  FILE* cert_file = fopen(cert_path, "rb");
  X509* existing_cert = PEM_read_X509(cert_file, NULL, NULL, NULL);
  fclose(cert_file);

  FILE* key_file = fopen(key_path, "rb");
  EVP_PKEY* existing_key = PEM_read_PrivateKey(key_file, NULL, NULL, NULL);
  fclose(key_file);

  /* set a few parameters of the certificate. */
  int days = atoi(validity_period);



  X509_gmtime_adj(X509_get_notBefore(x509), 0);
  X509_gmtime_adj(X509_get_notAfter(x509), days * 24 * 60 * 60);

  /* copy the existing certificate and modify the expiration date */
  X509_set_version(x509, X509_get_version(existing_cert));
  X509_set_pubkey(x509, X509_get_pubkey(existing_cert));
  X509_set_subject_name(x509, X509_get_subject_name(existing_cert));
  X509_set_issuer_name(x509, X509_get_issuer_name(existing_cert));

   X509_NAME *name_obj = X509_NAME_new();
    //Adding organization name
    if (org != NULL) {
        X509_NAME_add_entry_by_txt(name_obj, "C", MBSTRING_ASC, (unsigned char*)country, -1, -1, 0);
    }

    // Adding organizational unit name
    if (org_unit != NULL) {
        X509_NAME_add_entry_by_txt(name_obj, "ST", MBSTRING_ASC, (unsigned char*)state, -1, -1, 0);
    }

    // Adding locality name
    if (locality != NULL) {
        X509_NAME_add_entry_by_txt(name_obj, "L", MBSTRING_ASC, (unsigned char*)locality, -1, -1, 0);
    }

    // Adding state name
    if (state != NULL) {
        X509_NAME_add_entry_by_txt(name_obj, "O", MBSTRING_ASC, (unsigned char*)org, -1, -1, 0);
    }

    // Adding country name
    if (country != NULL) {
        X509_NAME_add_entry_by_txt(name_obj, "OU", MBSTRING_ASC, (unsigned char*)org_unit, -1, -1, 0);
    }



  ASN1_INTEGER_set(X509_get_serialNumber(x509), SN);
  X509_gmtime_adj(X509_get_notBefore(x509), 0);
  X509_gmtime_adj(X509_get_notAfter(x509), days * 24 * 60 * 60);

  /* sign the certificate with the existing private key */
  X509_sign(x509, pkey, EVP_sha256());

  EVP_PKEY_free(existing_key);
  X509_free(existing_cert);

  return x509;
}


 int  generateIERARHICCert( char * root_cert_file 
                ,  char * validity_period 
                ,  char *name 
                ,  char * issuer_name 
                ,  int  SN 
                ,  char * organization 
                ,  char *organizational_unit 
                ,  char *locality 
                ,  char *state 
                ,  char * country) {
    
    X509* root_cert;
    RSA* root_key;
    FILE* fp;
    int option_index = 0, c;
    EVP_PKEY* root_pkey ;


    // Private Key
        root_pkey = generate_key();
        FILE* pkey_file = fopen("IERARHICrootCA.key", "wb");
        PEM_write_PrivateKey(pkey_file, root_pkey, NULL, NULL, 0, NULL, NULL);
        fclose(pkey_file);

    // Root Certificate
    root_cert = generate_x509(root_pkey, validity_period, "Root CA", name, SN ,NULL,NULL,NULL,NULL,NULL);
    FILE* x509_file = fopen("IERARHICrootCA.crt", "wb");
    PEM_write_X509(x509_file, root_cert);
    fclose(x509_file);

    printf("\n ROOT DONE \n");

    // Intermediate Certificate
    
        // Private Key
        EVP_PKEY* intermediate_pkey = generate_key();
        pkey_file = fopen("IERARHICintermediateCA.key", "wb");
            
            PEM_write_PrivateKey(pkey_file, intermediate_pkey, NULL, NULL, 0, NULL, NULL);
            fclose(pkey_file);


        X509* intermediate_cert = Extend_CERT(intermediate_pkey, validity_period, "Intermediate CA","Root CA" , SN, 
        organization,organizational_unit,NULL,NULL,NULL,"IERARHICrootCA.crt", "IERARHICrootCA.key");
       
        FILE* intermediate_x509_file = fopen("IERARHICintermediateCA.crt", "wb");
        PEM_write_X509(intermediate_x509_file, intermediate_cert);
        fclose(intermediate_x509_file);


    printf("\n INTERMED DONE \n");


    // End-Entity Certificate

    EVP_PKEY* entity_pkey = generate_key();
    pkey_file = fopen("IERARHICendCA.key", "wb");
        
        PEM_write_PrivateKey(pkey_file, entity_pkey, NULL, NULL, 0, NULL, NULL);
        fclose(pkey_file);

    printf("\n SFASD DONE \n");

    X509* entity_cert = Extend_CERT(entity_pkey, validity_period, "End-Entity CA","Intermediate CA" , SN, 
    organization,organizational_unit,locality,state,country,"IERARHICintermediateCA.crt", "IERARHICintermediateCA.key");

    printf("\n UUUUUUUWWAA \n");

    FILE* entity_x509_file = fopen("IERARHICendCA.crt", "wb");
    PEM_write_X509(entity_x509_file, entity_cert);
    fclose(entity_x509_file);

    printf("\n END DONE \n");

    free(root_cert);
    free(intermediate_cert);
    free(entity_cert);

    free(root_pkey);
    free(intermediate_pkey);
    free(entity_pkey);

    return 0;

} 



int main(int argc, char *argv[]) {
    X509 *root_cert;
    RSA *root_key;
    FILE *fp;
    int option_index = 0, c;
    //EVP_PKEY *pkey = EVP_PKEY_new();


 // Private Key

  EVP_PKEY* pkey = generate_key();
  FILE* pkey_file = fopen("rootca.key", "wb");
  PEM_write_PrivateKey(pkey_file, pkey, NULL,NULL ,0 ,NULL ,NULL);
  fclose(pkey_file);

  EVP_PKEY_free(pkey);


    char *root_cert_file = NULL;
    char *validity_period = NULL;
    char *name = NULL;
    char *issuer_name = NULL;
    int  SN = 0;
    char * sn = NULL;
    char *organization = NULL;
    char *organizational_unit = NULL;
    char *locality = NULL;
    char *state = NULL;
    char *country = NULL;
    int ier = 0;
    char *IER =NULL;

    X509_CRL *crl = create_crl();  // CREATE REVOK LIST

    static struct option long_options[] = {
        {"root-cert", required_argument, 0, 'r'},
        {"validity-period", required_argument, 0, 'v'},
        {"name", required_argument, 0, 'n'},
        {"issuer-name", required_argument, 0, 'i'},
        {"serial-number", required_argument, 0 , 's' },
        {"organization", required_argument, 0, 'o'},
        {"organizational-unit", required_argument, 0, 'u'},
        {"locality", required_argument, 0, 'l'},
        {"state", required_argument, 0, 't'},
        {"country", required_argument, 0, 'y'},
        {"ierarhic",required_argument, 0, 'q'},
        {0, 0, 0, 0}
    };
    


    while ((c = getopt_long(argc, argv, "r:v:n:i:s:o:u:l:t:y:q:", long_options, &option_index)) != -1) {
        switch (c) {
            case 'r':
                root_cert_file = optarg;
                Sanitise(root_cert_file);
                break;
            case 'v':
                validity_period = optarg;
                Sanitise(validity_period);
                break;
            case 'n':
                name = optarg;
                Sanitise(name);
                break;
            case 'i':
                issuer_name = optarg;
                Sanitise(issuer_name);
                break;
            case 's':
                sn = optarg;
                Sanitise(sn);
                SN = atoi(sn);
                break;
            case 'o':
                organization = optarg;
                Sanitise(organization);
                break;
            case 'u':
                organizational_unit = optarg;
                Sanitise(organizational_unit);
                break;
            case 'l':
                locality = optarg;
                Sanitise(locality);
                break;
            case 't':
                state = optarg;
                Sanitise(state);
                break;
            case 'y':
                country = optarg;
                Sanitise(country);
                break;
            case 'q':
                IER = optarg;
                Sanitise(IER);
                ier = atoi(IER);
                break;
            default:
                printf("\n Usage: %s --root-cert=<file> \n --validity-period=<days> \n --name=<name> \n --issuer-name=<issuer> \n\
                --serial-number=<long> \n [--organization=<organization>] \n [--organizational-unit=<unit>] \n [--locality=<locality>]\n\
                [--state=<state>] \n [--country=<country>] \n --ierarhic=<0/1>\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    // Verificarea argumentelor de linie de comandă
   if (!root_cert_file || !validity_period || !name || !issuer_name || SN == 0 || 
       !organization || !organizational_unit || !locality || !state || !country) {
         printf("\n Usage: %s --root-cert=<file> \n --validity-period=<days> \n --name=<name> \n --issuer-name=<issuer> \n\
                --serial-number=<long> \n [--organization=<organization>] \n [--organizational-unit=<unit>] \n [--locality=<locality>]\n\
                [--state=<state>] \n [--country=<country>] \n --ierarhic=<0/1>\n", argv[0]);

                printf("\n root_cert_file = %s , validity_period = %s , name = %s , issuer_name = %s , serial-number = %d \n \
                        organization = %s , organizational_unit = %s , locality = %s , state = %s , country = %s \n "
                , root_cert_file 
                , validity_period 
                , name 
                , issuer_name 
                , SN 
                , organization 
                , organizational_unit 
                , locality 
                , state 
                ,country);
                exit(EXIT_FAILURE);
    }


        if (ier == 0){

    EVP_PKEY* pkeyPair = generate_key();
    X509* x509 = generate_x509(pkeyPair,validity_period,name,issuer_name,SN,
     organization , organizational_unit ,locality ,state , country);
    
    FILE* x509_file = fopen("rootca.crt", "wb");
    PEM_write_X509(x509_file, x509);
    fclose(x509_file);

    printf("\n --CERTIFICATE CREATED-- \n");
    printf("\n --CERT = %s-- \n","rootca.crt");
    printf("\n --SIGNATURE = %s-- \n","rootca.key");
    
    isCertificateValid(x509,crl);

   

     // Extend the certificate 

    X509 * NewCert =  Extend_CERT(pkey,"10",name,issuer_name,SN,organization,organizational_unit,locality,state,country,"rootca.crt","rootca.key");
    
    FILE* x509_file2 = fopen("new_rootca.crt", "wb");
    PEM_write_X509(x509_file2, NewCert);
    fclose(x509_file2);

    printf("\n --CERTIFICATE EXTENDED- \n");
    printf("\n --CERT = %s-- \n","new_rootca.crt");
    printf("\n --SIGNATURE = %s-- \n","rootca.key");
    
    isCertificateValid(NewCert,crl);

    
    // Add a revoked certificate to the CRL

    X509* cert = x509; // get the certificate to be revoked
    EVP_PKEY* issuer_key = pkey; 
    int serial = SN; 
    char* reason = "Non Compliance"; 

    
    Revoc_cert(crl,cert, issuer_key, serial, reason);

    printf("\n --CERTIFICATE REVOKED-- \n");
    printf("\n --CERT = %s-- \n","rootca.crt");
    printf("\n --SIGNATURE = %s-- \n","rootca.key");

    isCertificateValid(x509,crl);



        }
        else {

            generateIERARHICCert( root_cert_file 
                , validity_period 
                , name 
                , issuer_name 
                , SN 
                , organization 
                , organizational_unit 
                , locality 
                , state 
                , country );

            printf("\n ----IERARHIC CERTIFICATE EXTENDED---- \n");

            printf("\n -ROOT CERTIFICATE CREATED- \n");
            printf("\n -CERT = %s- \n","IERARHICrootCA.crt");
            printf("\n -SIGNATURE = %s- \n","IERARHICrootCA.crt");

            printf("\n --INTERMEDIATE CERTIFICATE CREATED-- \n");
            printf("\n --CERT = %s-- \n","IERARHICintermediateCA.crt");
            printf("\n --SIGNATURE = %s-- \n","IERARHICintermediateCA.key");
            
            printf("\n ---END-ENTITY CERTIFICATE CREATED--- \n");
            printf("\n ---CERT = %s-- \n","IERARHICendCA.crt");
            printf("\n ---SIGNATURE = %s-- \n","IERARHICendCA.key");



        }



    

    // free(root_cert_file);
    // free(validity_period);
    // free(name);
    // free(issuer_name);
    // free(organization);
    // free(organizational_unit);
    // free(locality);
    // free(state);
    // free(country);
    // free(sn);

    return 0;
}