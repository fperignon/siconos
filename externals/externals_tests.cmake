include(tools4tests)

# wrapper are not needed
set(TEST_WRAP)

if(WITH_${COMPONENT}_TESTING)
  
  BEGIN_TEST(netlib/odepack/test)
  IF(NOT USE_SANITIZER)
    NEW_TEST(odepacktest1 DLSODES-test.f)
    NEW_TEST(odepacktest2 DLSODAR-test.f)
    NEW_TEST(odepacktest3 DLSODI-test.f)
    NEW_TEST(odepacktest4 DLSODPK-test.f)
    NEW_TEST(odepacktest5 DLSODA-test.f)
    NEW_TEST(odepacktest6 DLSODE-test.f)
    NEW_TEST(odepacktest7 DLSODIS-test.f)
    NEW_TEST(odepacktest8 DLSODKR-test.f)
    NEW_TEST(odepacktest9 DLSOIBT-test.f)
  ENDIF(NOT USE_SANITIZER)
  if(WITH_CXX)
    NEW_TEST(odepacktest10 test-funcC-inC.cpp funC.cpp)
  endif()
  END_TEST()

  IF(NOT USE_SANITIZER)
    BEGIN_TEST(hairer/test)
    NEW_TEST(dr_iso1 dr_iso.f)
    NEW_TEST(dr_iso1sp dr_isosp.f)
    NEW_TEST(dr1_radau5 dr1_radau5.f)
    NEW_TEST(dr2_radau5 dr2_radau5.f)
    NEW_TEST(dr_radau dr_radau.f)
    NEW_TEST(dr_radaup dr_radaup.f)
    NEW_TEST(dr_rodas dr_rodas.f)
    NEW_TEST(dr_seulex dr_seulex.f)
    END_TEST()
  ENDIF(NOT USE_SANITIZER)


endif()
