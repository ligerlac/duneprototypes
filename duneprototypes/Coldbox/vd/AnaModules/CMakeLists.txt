cet_build_plugin(TrackWireROIDump art::module
                 lardataobj_RawData
                 lardataobj_RecoBase
                 protoduneana::ProtoDUNEUtilities
                 ART_FRAMEWORK_CORE
                 ART_FRAMEWORK_PRINCIPAL
                 ART_FRAMEWORK_SERVICES_REGISTRY
                 ART_ROOT_IO_TFILE_SUPPORT
                 ROOT::Core
                 ART_ROOT_IO_TFILESERVICE_SERVICE
                 ART_PERSISTENCY_PROVENANCE
                 messagefacility::MF_MessageLogger
                 CETLIB
                 ROOT_BASIC_LIB_LIST
                 BASENAME_ONLY
)

install_fhicl()
install_source()
