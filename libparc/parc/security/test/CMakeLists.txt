set(TestsExpectedToPass
  test_parc_Certificate
  test_parc_CertificateFactory
  test_parc_CertificateType
  test_parc_ContainerEncoding
  test_parc_CryptoCache
  test_parc_CryptoHash
  test_parc_CryptoHashType
  test_parc_CryptoHasher
  test_parc_CryptoSuite
  test_parc_DiffieHellman
  test_parc_DiffieHellmanKeyShare
  test_parc_Identity
  test_parc_IdentityFile
  test_parc_InMemoryVerifier
  test_parc_InMemoryVerifierECDSA
  test_parc_Key
  test_parc_KeyId
  test_parc_KeyStore
  test_parc_SecureRandom
  test_parc_Pkcs12KeyStore
  test_parc_Pkcs12KeyStoreECDSA
  test_parc_PublicKeySigner
  test_parc_PublicKeyECSigner
  test_parc_SymmetricKeySigner
  test_parc_SymmetricKeyStore
  test_parc_Security
  test_parc_Signature
  test_parc_Signer
  test_parc_SignerEC
  test_parc_SigningAlgorithm
  test_parc_Verifier
  test_parc_X509Certificate
  )

set(EXTRA_DATA_FILES
	README.digests
	README.keystore
	README.symmetric
	test_crt.der
	test_crt_der.bin
	test_crt_sha256.bin
	test_der.bin
	test_digest_bytes_128.bin
	test_digest_bytes_128.sha256
	test_digest_bytes_128.sha512
	test.pem
	test_pubkey.bin
	test_pubkey.der
	test_pubkey.pem
	test_random_bytes
	test_random_bytes.sig
	test_random_bytes.sig_ec
	test_random_bytes.hmac_sha256
	test_random_bytes.hmac_sha512
	test_rsa.p12
	test_rsa_crt.der
	test_rsa_crt_sha256.bin
	test_rsa_key.der
	test_rsa_key.pem
	test_rsa_pub.der
	test_rsa_pub.pem
	test_rsa_pub_sha256.bin
	test_symmetric_key.bin
	test_symmetric_key.sha256
	test_ec.p12
	test_ec_crt.der
	test_ec_crt_sha256.bin
	test_ec_key.der
	test_ec_key.pem
	test_ec_pub.der
	test_ec_pub.pem
	test_ec_pub_sha256.bin
	)

foreach(data_file ${EXTRA_DATA_FILES})
  configure_file(${data_file} ${data_file} COPYONLY)
endforeach()


# Enable gcov output for the tests
add_definitions(--coverage)
set(CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS} " --coverage")


foreach(test ${TestsExpectedToPass})
   AddTest(${test})
endforeach()
