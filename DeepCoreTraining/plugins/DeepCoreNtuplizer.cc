// -*- C++ -*-
//
// Package:    RecoTracker/DeepCoreTraining
// Class:      DeepCoreNtuplizer
//
/**\class DeepCoreNtuplizer DeepCoreNtuplizer.cc RecoTracker/DeepCoreTraining/plugins/DeepCoreNtuplizer.cc

Description: [one line class summary]

Implementation:
[Notes on implementation]
*/
//
// Original Author:  Valerio Bertacchi
//         Created:  Fri, 09 Jul 2021 10:43:34 GMT
//
//

// system include files
#include <memory>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/stream/EDProducer.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/StreamID.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"



#include "Geometry/CommonDetUnit/interface/GlobalTrackingGeometry.h"
#include "Geometry/Records/interface/GlobalTrackingGeometryRecord.h"
#include "Geometry/CommonDetUnit/interface/GlobalTrackingGeometry.h"
#include "Geometry/Records/interface/TrackerTopologyRcd.h"

#include "DataFormats/TrackerCommon/interface/TrackerTopology.h"
#include "DataFormats/Common/interface/Handle.h"
#include "DataFormats/Common/interface/DetSetVector.h"
#include "DataFormats/Common/interface/DetSet.h"
#include "DataFormats/SiPixelCluster/interface/SiPixelCluster.h"
#include "DataFormats/VertexReco/interface/Vertex.h"
#include "DataFormats/TrackReco/interface/Track.h"
#include "DataFormats/VertexReco/interface/VertexFwd.h"
#include "DataFormats/JetReco/interface/Jet.h"
#include "DataFormats/SiPixelDigi/interface/PixelDigi.h"
#include "DataFormats/GeometryVector/interface/VectorUtil.h"
#include "DataFormats/SiPixelDetId/interface/PXBDetId.h"
#include "DataFormats/Math/interface/Point3D.h"
#include "DataFormats/Math/interface/Vector3D.h"
#include "DataFormats/Candidate/interface/Candidate.h"
#include "SimDataFormats/TrackingHit/interface/PSimHit.h"

#include "RecoLocalTracker/ClusterParameterEstimator/interface/PixelClusterParameterEstimator.h"
#include "RecoLocalTracker/Records/interface/TkPixelCPERecord.h"

#include "SimDataFormats/TrackerDigiSimLink/interface/PixelDigiSimLink.h"

#include "TrackingTools/GeomPropagators/interface/StraightLinePlaneCrossing.h"
#include "TrackingTools/GeomPropagators/interface/Propagator.h"
#include "TrackingTools/Records/interface/TrackingComponentsRecord.h"

#include "MagneticField/Engine/interface/MagneticField.h"
#include "MagneticField/Records/interface/IdealMagneticFieldRecord.h"



#include <boost/range.hpp>
#include <boost/foreach.hpp>
#include "boost/multi_array.hpp"

#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"

//#include "SimG4Core/Notification/interface/G4SimTrack.h"
#include "SimDataFormats/Track/interface/SimTrack.h"

// #include "SimDataFormats/Vertex/interface/SimVertex.h"


#include "Geometry/CommonDetUnit/interface/PixelGeomDetUnit.h"



#include "TTree.h"

#include <tuple>
//
// class declaration
//

class DeepCoreNtuplizer : public edm::stream::EDProducer<> {
    public:
        explicit DeepCoreNtuplizer(const edm::ParameterSet&);
        ~DeepCoreNtuplizer();

        static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

        struct TrackAndState
        {
            TrackAndState(const reco::Track *aTrack, TrajectoryStateOnSurface aState) :
                track(aTrack), state(aState) {}
            const reco::Track*      track;
            TrajectoryStateOnSurface state;
        };

        template<typename Cluster>
            struct ClusterWithTracks
            {
                ClusterWithTracks(const Cluster &c) : cluster(&c) {}
                const Cluster* cluster;
                std::vector<TrackAndState> tracks;
            };

        typedef ClusterWithTracks<SiPixelCluster> SiPixelClusterWithTracks;

        typedef boost::sub_range<std::vector<SiPixelClusterWithTracks> > SiPixelClustersWithTracks;

        TFile* DeepCoreNtuplizer_out;
        TTree* DeepCoreNtuplizerTree;
        int eventID;
        
        double caloJetP,caloJetPt,caloJetEta,caloJetPhi;
        std::vector<double> pixelXvec,pixelYvec,pixelZvec,pixelEtavec,pixelPhivec,pixelChargevec, pixelSimTrackPtvec, pixelSimTrackEtavec, pixelSimTrackPhivec;
        std::vector<int> pixelTrackerLayervec,pixelSimTrackIDvec, pixelSimTrackPdgvec;


    private:
        void produce(edm::Event&, const edm::EventSetup&) override;

        // ----------member data ---------------------------

        std::string propagatorName_;
        //  edm::ESHandle<MagneticField>          magfield_;
        //  edm::ESHandle<GlobalTrackingGeometry> geometry_.;
        //  edm::ESHandle<Propagator>             propagator_;

        //Migration to ESGetToken from ESHandle
        edm::ESGetToken<MagneticField, IdealMagneticFieldRecord>          magfield_;
        edm::ESGetToken<GlobalTrackingGeometry, GlobalTrackingGeometryRecord> geometryToken_;
        edm::ESGetToken<Propagator,TrackingComponentsRecord>             propagator_;

        edm::ESGetToken<PixelClusterParameterEstimator, TkPixelCPERecord> parEstHandle;
        edm::ESGetToken<TrackerTopology, TrackerTopologyRcd> tTopoHandle; 

        edm::EDGetTokenT<std::vector<reco::Vertex> > vertices_;
        edm::EDGetTokenT<edmNew::DetSetVector<SiPixelCluster> > pixelClusters_;
        std::vector<SiPixelClusterWithTracks> allSiPixelClusters;
        std::map<uint32_t, SiPixelClustersWithTracks> siPixelDetsWithClusters;
        edm::Handle<edmNew::DetSetVector<SiPixelCluster> > inputPixelClusters;
        edm::EDGetTokenT<edm::View<reco::Candidate> > cores_;
        edm::EDGetTokenT<std::vector<SimTrack> > simtracksToken;
        edm::EDGetTokenT<std::vector<PSimHit> > PSimHitToken;
        edm::Handle<std::vector<PSimHit> > simhits;
        edm::EDGetTokenT<std::vector<PSimHit> > PSimHitECToken;
        edm::Handle<std::vector<PSimHit> > simhitsEC;



        const std::vector<SimTrack> *simtracksVector;


        edm::EDGetTokenT<edm::DetSetVector<PixelDigiSimLink> > pdslToken;
        edm::Handle< edm::DetSetVector<PixelDigiSimLink> > PDSLContainer;
        const edm::DetSetVector<PixelDigiSimLink> *pixelSimLinks;



        double ptMin_;
        double pMin_;
        double deltaR_;
        double chargeFracMin_;
        double centralMIPCharge_;
        std::string pixelCPE_;

        bool barrelTrain_;
        bool endcapTrain_;
        bool fullTrain_;





        std::pair<int,int> local2Pixel(double, double, const GeomDet*);

        LocalPoint pixel2Local(int, int, const GeomDet*);

        // Pixel size fix:
        // std::pair<bool,bool> pixel2Size(int, int, const GeomDet*);

        void getSimTracksFromPixel(SiPixelCluster::Pixel pixel, unsigned int detId, std::vector<unsigned int> & uniqueSimTrackIds);
        //int getOneSimTrackFromPixel(SiPixelCluster::Pixel pixel, unsigned int detId);
        std::tuple<int,int,float,float,float> getOneSimTrackFromPixel(SiPixelCluster::Pixel pixel, unsigned int detId);

};

DeepCoreNtuplizer::DeepCoreNtuplizer(const edm::ParameterSet& iConfig) :
    magfield_(esConsumes<MagneticField, IdealMagneticFieldRecord>()),
    geometryToken_(esConsumes<GlobalTrackingGeometry, GlobalTrackingGeometryRecord>()),
    propagator_(esConsumes<Propagator,TrackingComponentsRecord>()),

    parEstHandle(esConsumes(edm::ESInputTag("", iConfig.getParameter<std::string>("pixelCPE")))),
    tTopoHandle(esConsumes<TrackerTopology, TrackerTopologyRcd>()),

    vertices_(consumes<reco::VertexCollection>(iConfig.getParameter<edm::InputTag>("vertices"))),
    pixelClusters_(consumes<edmNew::DetSetVector<SiPixelCluster> >(iConfig.getParameter<edm::InputTag>("pixelClusters"))),
    cores_(consumes<edm::View<reco::Candidate> >(iConfig.getParameter<edm::InputTag>("cores"))),
    simtracksToken(consumes<std::vector<SimTrack> >(iConfig.getParameter<edm::InputTag>("simTracks"))),
    PSimHitToken(consumes<std::vector<PSimHit> >(iConfig.getParameter<edm::InputTag>("simHit"))),
    PSimHitECToken(consumes<std::vector<PSimHit> >(iConfig.getParameter<edm::InputTag>("simHitEC"))),
    pdslToken(consumes<edm::DetSetVector<PixelDigiSimLink> >(iConfig.getParameter<edm::InputTag>("PixelDigiSimLinkVector"))),
    ptMin_(iConfig.getParameter<double>("ptMin")),
    pMin_(iConfig.getParameter<double>("pMin")),
    deltaR_(iConfig.getParameter<double>("deltaR")),
    chargeFracMin_(iConfig.getParameter<double>("chargeFractionMin")),
    centralMIPCharge_(iConfig.getParameter<double>("centralMIPCharge")),
    pixelCPE_(iConfig.getParameter<std::string>("pixelCPE")),
    barrelTrain_(iConfig.getParameter<bool>("barrelTrain")),
    endcapTrain_(iConfig.getParameter<bool>("endcapTrain")),
    fullTrain_(iConfig.getParameter<bool>("fullTrain"))
{

    //  usesResource("TFileService");
    edm::Service<TFileService> fileService;

    DeepCoreNtuplizerTree= fileService->make<TTree>("tree","tree");
    
    //Global variables
    DeepCoreNtuplizerTree->Branch("event",&eventID);
    DeepCoreNtuplizerTree->Branch("caloJetP",&caloJetP);
    DeepCoreNtuplizerTree->Branch("caloJetPt",&caloJetPt);
    DeepCoreNtuplizerTree->Branch("caloJetEta",&caloJetEta);
    DeepCoreNtuplizerTree->Branch("caloJetPhi",&caloJetPhi);

    //pixel variables
    DeepCoreNtuplizerTree->Branch("pixelX",&pixelXvec);
    DeepCoreNtuplizerTree->Branch("pixelY",&pixelYvec);
    DeepCoreNtuplizerTree->Branch("pixelZ",&pixelZvec);
    DeepCoreNtuplizerTree->Branch("pixelCharge",&pixelChargevec);
    DeepCoreNtuplizerTree->Branch("pixelEta",&pixelEtavec);
    DeepCoreNtuplizerTree->Branch("pixelPhi",&pixelPhivec);
    DeepCoreNtuplizerTree->Branch("pixelTrackerLayer",&pixelTrackerLayervec);
    
    //truth labeling
    DeepCoreNtuplizerTree->Branch("pixelSimTrackID",&pixelSimTrackIDvec);
    DeepCoreNtuplizerTree->Branch("pixelSimTrackPdg",&pixelSimTrackPdgvec);
    DeepCoreNtuplizerTree->Branch("pixelSimTrackPt",&pixelSimTrackPtvec);
    DeepCoreNtuplizerTree->Branch("pixelSimTrackEta",&pixelSimTrackEtavec);
    DeepCoreNtuplizerTree->Branch("pixelSimTrackPhi",&pixelSimTrackPhivec);




}

DeepCoreNtuplizer::~DeepCoreNtuplizer() {
    // do anything here that needs to be done at destruction time
    // (e.g. close files, deallocate resources etc.)
    //
    // please remove this method altogether if it would be left empty
}

//
// member functions
//

// ------------ method called for each event  ------------
#define foreach BOOST_FOREACH

// ------------ method called to produce the data  ------------
void DeepCoreNtuplizer::produce(edm::Event& iEvent, const edm::EventSetup& iSetup) {
    using namespace edm;

    eventID= iEvent.id().event();

    const GlobalTrackingGeometry* geometry_ = &iSetup.getData(geometryToken_);


    iEvent.getByToken(pixelClusters_, inputPixelClusters);
    allSiPixelClusters.clear(); siPixelDetsWithClusters.clear();
    allSiPixelClusters.reserve(inputPixelClusters->dataSize()); // this is important, otherwise push_back invalidates the iterators

    edm::Handle<std::vector<SimTrack> > simtracks;
    iEvent.getByToken(simtracksToken, simtracks);

    Handle<std::vector<reco::Vertex> > vertices;
    iEvent.getByToken(vertices_, vertices);

    iEvent.getByToken(PSimHitToken, simhits);
    iEvent.getByToken(PSimHitECToken, simhitsEC);


    Handle<edm::View<reco::Candidate> > cores;
    iEvent.getByToken(cores_, cores);

    iEvent.getByToken(pdslToken, PDSLContainer);
    pixelSimLinks = PDSLContainer.product();

    
    
    simtracksVector = simtracks.product();

    const PixelClusterParameterEstimator* parEst = &iSetup.getData(parEstHandle);  

    const TrackerTopology* tTopo = &iSetup.getData(tTopoHandle);


    auto output = std::make_unique<edmNew::DetSetVector<SiPixelCluster>>();



    //Go go graphcore!
    //edmNew::DetSetVector<SiPixelCluster>::const_iterator clusterDetSetVecItr = inputPixelClusters->begin();

    for (unsigned int jetIdx = 0; jetIdx < cores->size(); jetIdx++) { //loop jet
        const reco::Candidate& jet = (*cores)[jetIdx];
        if ((jet.pt() > ptMin_ && std::abs(jet.eta())< 1.8) || (jet.p()>ptMin_ && std::abs(jet.eta())>1.8)) { 
            caloJetP = jet.p();
            caloJetPt = jet.pt();
            caloJetEta = jet.eta();
            caloJetPhi = jet.phi();

            //std::cout << "Jet! " << jet.eta() << " " << jet.phi() << std::endl;
            const reco::Vertex& jetVertex = (*vertices)[0];
            GlobalPoint jetVertexPoint(jetVertex.position().x(), jetVertex.position().y(), jetVertex.position().z());
            //std::cout << "Jet Vertex Position " << jetVertexPoint.eta() << " " <<jetVertexPoint.phi() << " z=" << jetVertexPoint.z() << std::endl;
            GlobalVector jetDirection(jet.px(), jet.py(), jet.pz());

            int nPixels=0;
            for ( edmNew::DetSetVector<SiPixelCluster>::const_iterator clusterDetSetVecItr = inputPixelClusters->begin(); clusterDetSetVecItr != inputPixelClusters->end(); clusterDetSetVecItr++) { //loop detsets of clusters
                const edmNew::DetSet<SiPixelCluster>& DetSetOfClusters = *clusterDetSetVecItr;
                const GeomDet* det = geometry_->idToDet(DetSetOfClusters.id()); 
                for (auto cluster = DetSetOfClusters.begin(); cluster != DetSetOfClusters.end(); cluster++) {//loop clusters (finally)

                    GlobalPoint clusterPos = det->surface().toGlobal(parEst->localParametersV(*cluster,(*geometry_->idToDetUnit(DetSetOfClusters.id())))[0].first);//cluster position, as estimated from pixels. Needed to compare to jet direction
                    GlobalVector clusterDirection = clusterPos - jetVertexPoint;
                    if (Geom::deltaR(jetDirection, clusterDirection) < deltaR_) {
                        //std::cout << "\tfound cluster" << clusterPos.eta() << " " << clusterPos.phi() << " " << clusterPos.z() << " deltaR=" << Geom::deltaR(jetDirection, clusterDirection) << std::endl; 
                        //get global positions of every pixel in cluster
                        for(int pixel_i=0; pixel_i<cluster->size(); pixel_i++){
                            SiPixelCluster::Pixel pixel=cluster->pixel(pixel_i);
                            LocalPoint pixelLocalPoint = pixel2Local(pixel.x,pixel.y,det);
                            GlobalPoint pixelGlobalPoint = det->toGlobal(pixelLocalPoint);
                            if(fabs(pixelGlobalPoint.phi())>0) nPixels++;
                            
                            int pixelLayer = tTopo->layer(det->geographicalId());
                            if(det->geographicalId().subdetId()==PixelSubdetector::PixelEndcap) {
                                pixelLayer+=4; //endcap layer counting = 5,6,7
                            }
                            //std::cout << "\t\tpixel: " << pixelGlobalPoint.eta() << " " << pixelGlobalPoint.phi() << " " << pixelGlobalPoint.z() << " " << pixelLayer << "\t" << (pixel.adc)/(float) (14000) << std::endl;
                           
                            
                            //TRUTH: for a given pixel, which simtrack left the most charge in it?
                            std::tuple<int,int,float,float,float> simTrackIdPdgPtEtaPhi;
                            simTrackIdPdgPtEtaPhi = DeepCoreNtuplizer::getOneSimTrackFromPixel( pixel, DetSetOfClusters.id());

                            //FILL
                            pixelXvec.push_back(pixelGlobalPoint.x());
                            pixelYvec.push_back(pixelGlobalPoint.y());
                            pixelZvec.push_back(pixelGlobalPoint.z());
                            pixelEtavec.push_back(pixelGlobalPoint.eta());
                            pixelPhivec.push_back(pixelGlobalPoint.phi());
                            pixelTrackerLayervec.push_back(pixelLayer);
                            pixelChargevec.push_back((pixel.adc)/(float)(14000));

                            pixelSimTrackIDvec.push_back( std::get<0>(simTrackIdPdgPtEtaPhi));
                            pixelSimTrackPdgvec.push_back(std::get<1>(simTrackIdPdgPtEtaPhi));
                            pixelSimTrackPtvec.push_back( std::get<2>(simTrackIdPdgPtEtaPhi));
                            pixelSimTrackEtavec.push_back(std::get<3>(simTrackIdPdgPtEtaPhi));
                            pixelSimTrackPhivec.push_back(std::get<4>(simTrackIdPdgPtEtaPhi));
                        }
                    }
                }
            }
            //std::cout << "tot pixels: " << nPixels << std::endl; //50-300 pixels within deltaR=0.1. Too many?
            
            if(nPixels>0) DeepCoreNtuplizerTree->Fill();
            pixelXvec.clear();
            pixelYvec.clear();
            pixelZvec.clear();
            pixelEtavec.clear();
            pixelPhivec.clear();
            pixelTrackerLayervec.clear();
            pixelChargevec.clear();
            
            pixelSimTrackIDvec.clear();
            pixelSimTrackPdgvec.clear();
            pixelSimTrackPtvec.clear();
            pixelSimTrackEtavec.clear();
            pixelSimTrackPhivec.clear();
        }
    }
}

void DeepCoreNtuplizer::getSimTracksFromPixel(SiPixelCluster::Pixel pixel, unsigned int detId, std::vector<unsigned int> & uniqueSimTrackIds){
    
    //find all simtracks depositing charge in pixel (usually only one)
    auto firstLink = pixelSimLinks->find(detId);
    if(firstLink != pixelSimLinks->end()){
        auto link_detset = (*firstLink);
        for(auto linkiter : link_detset.data){
            std::pair<int,int> pos = PixelDigi::channelToPixel(linkiter.channel());

            if(pos.first == pixel.x && pos.second == pixel.y){
                std::cout << " fraction " << linkiter.fraction() << std::endl;
                for(auto iter = simtracksVector->begin(); iter != simtracksVector->end(); ++iter){
                    
                    if(iter->trackId() == linkiter.SimTrackId() && iter->momentum().Pt()>0.2 && iter->momentum().Pt()<99999){ //cut on P() instead?
                        bool isNew = true;
                        for(auto iD : uniqueSimTrackIds){
                            if(iD==(unsigned int)(iter->trackId())) isNew=false;
                        }
                        if(isNew){
                            uniqueSimTrackIds.push_back((unsigned int)(iter->trackId()));
                            std::cout << "SimTrack pdgId: " << iter->type() << " unique ID: " << iter->trackId()  << std::endl;
                            //simTrackPDGIds.push_back((signed int) iter->pdgId());
                        }
                    }
                }
            }
        }
    } 
}

std::tuple<int,int,float,float,float> DeepCoreNtuplizer::getOneSimTrackFromPixel(SiPixelCluster::Pixel pixel, unsigned int detId){
    
    std::tuple<int,int,float,float,float> simTrackIdPdgPtEtaPhi;
    
    std::get<0>(simTrackIdPdgPtEtaPhi)=-99;
    std::get<1>(simTrackIdPdgPtEtaPhi)=-99;
    std::get<2>(simTrackIdPdgPtEtaPhi)=-99;
    std::get<3>(simTrackIdPdgPtEtaPhi)=-99;
    std::get<4>(simTrackIdPdgPtEtaPhi)=-99;
     
    //store simtrack ID with greatest charge in pixel
    float maxChargeFraction=0.0;
    //int maxChargeSimTrackID=0;
    auto firstLink = pixelSimLinks->find(detId);
    if(firstLink != pixelSimLinks->end()){
        auto link_detset = (*firstLink);

        for(auto linkiter : link_detset.data){
            std::pair<int,int> pos = PixelDigi::channelToPixel(linkiter.channel());

            if(pos.first == pixel.x && pos.second == pixel.y){
                for(auto iter = simtracksVector->begin(); iter != simtracksVector->end(); ++iter){
                    if(iter->trackId() == linkiter.SimTrackId() && iter->momentum().Pt()>0.2 && iter->momentum().Pt()<99999){ //cut on P() instead?
                        if(linkiter.fraction()>maxChargeFraction){
                            maxChargeFraction=linkiter.fraction();
                            //maxChargeSimTrackID=iter->trackId();

                            std::get<0>(simTrackIdPdgPtEtaPhi)=iter->trackId();
                            std::get<1>(simTrackIdPdgPtEtaPhi)=iter->type();
                            std::get<2>(simTrackIdPdgPtEtaPhi)=iter->momentum().Pt();
                            std::get<3>(simTrackIdPdgPtEtaPhi)=iter->momentum().eta();
                            std::get<4>(simTrackIdPdgPtEtaPhi)=iter->momentum().phi();

                        }
                    }
                }
            }
        }
    }
    return simTrackIdPdgPtEtaPhi;
}

std::pair<int,int> DeepCoreNtuplizer::local2Pixel(double locX, double locY, const GeomDet* det){
    LocalPoint locXY(locX,locY);
    float pixX=(dynamic_cast<const PixelGeomDetUnit*>(det))->specificTopology().pixel(locXY).first;
    float pixY=(dynamic_cast<const PixelGeomDetUnit*>(det))->specificTopology().pixel(locXY).second;
    std::pair<int, int> out(pixX,pixY);
    return out;
}

LocalPoint DeepCoreNtuplizer::pixel2Local(int pixX, int pixY, const GeomDet* det){
    float locX=(dynamic_cast<const PixelGeomDetUnit*>(det))->specificTopology().localX(pixX);
    float locY=(dynamic_cast<const PixelGeomDetUnit*>(det))->specificTopology().localY(pixY);
    LocalPoint locXY(locX,locY);
    return locXY;
}


// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
void DeepCoreNtuplizer::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
    //The following says we do not know what parameters are allowed so do no validation
    // Please change this to state exactly what you do use, even if it is no parameters
    edm::ParameterSetDescription desc;

    desc.add<edm::InputTag>("vertices", edm::InputTag("offlinePrimaryVertices"));
    desc.add<edm::InputTag>("pixelClusters", edm::InputTag("siPixelClustersPreSplitting"));
    desc.add<edm::InputTag>("cores", edm::InputTag("ak4CaloJets"));
    desc.add<edm::InputTag>("simTracks", edm::InputTag("g4SimHits"));
    desc.add<edm::InputTag>("simHit", edm::InputTag("TrackerHitsPixelBarrelLowTof"));
    desc.add<edm::InputTag>("simHitEC", edm::InputTag("TrackerHitsPixelEndcapLowTof"));
    desc.add<double>("ptMin", 250);
    desc.add<double>("pMin", 0);
    desc.add<double>("deltaR", 0.1);
    desc.add<double>("centralMIPCharge", 18000.0);
    desc.add<double>("chargeFractionMin", 2);
    desc.add<std::string>("pixelCPE", "PixelCPEGeneric");
    desc.add<bool>("barrelTrain", true);
    desc.add<bool>("endcapTrain", false);
    desc.add<bool>("fullTrain", false);
    desc.setUnknown();
    descriptions.addDefault(desc);
}

//define this as a plug-in
DEFINE_FWK_MODULE(DeepCoreNtuplizer);
