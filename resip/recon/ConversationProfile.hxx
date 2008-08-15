#if !defined(ConversationProfile_hxx)
#define ConversationProfile_hxx

#include <resip/dum/UserProfile.hxx>
#include <resip/stack/SdpContents.hxx>

namespace useragent
{

/**
  This class extends the resip UserProfile to include Conversation 
  Manager specific settings.

  Author: Scott Godin (sgodin AT SipSpectrum DOT com)
*/

class ConversationProfile : public resip::UserProfile
{
public:  
   ConversationProfile(); 

   enum NatTraversalMode
   {
      NoNatTraversal,
      StunBindDiscovery,
      TurnUdpAllocation,
      TurnTcpAllocation,
      TurnTlsAllocation
   };

   enum SecureMediaMode
   {
      NoSecureMedia, // Will accept secure media offers, but will not offer secure media in calls placed
      Srtp,       // SRTP with keying outside of media stream - ie. SDES key negotiation via SDP
      SrtpDtls    // SRTP with DTLS key negotiation
   };

   enum SecureMediaCryptoSuite
   {
      SRTP_AES_CM_128_HMAC_SHA1_32,
      SRTP_AES_CM_128_HMAC_SHA1_80
   };

   /**
     Constructor

     @param baseProfile if any UserProfile settings are not set on this
                        class, then the setting will "fall through" to 
                        the base profile
   */  
   ConversationProfile(resip::SharedPtr<resip::Profile> baseProfile);  

   /**
     Get/Set the session capabilities.  Session capabilites are expressed
     as a SessionDesriptionProtocol body.  Typically this is populated as you
     would expect an SDP offer to look like that is generated by this application.

     This is used to generate SDP offers and form valid SDP answers.

     @return SdpContents representing the session capabilities
   */  
   virtual resip::SdpContents& sessionCaps();
   virtual const resip::SdpContents sessionCaps() const; 

   /**
     Get/Set wether auto answers are allowed, if an autoanswer indication
     is found in a new inbound call request.

     @return bool True if auto answer is enabled
   */  
   virtual bool& allowAutoAnswer() { return mAllowAutoAnswer; }
   virtual const bool allowAutoAnswer() const { return mAllowAutoAnswer; }

   /**
     Get/Set wether priority auto answers are allowed, if a priority 
     autoanswer indication is found in a new inbound call request.

     @note Priority Auto Answers are indicated using the PrivAnswerMode
           SIP header

     @return bool True if priority auto answer is enabled
   */  
   virtual bool& allowPriorityAutoAnswer() { return mAllowPriorityAutoAnswer; }
   virtual const bool allowPriorityAutoAnswer() const { return mAllowPriorityAutoAnswer; }

   /** 
     Checks an Invite message for auto-answer headers against the current allow 
     settings.  
     
     @param inviteRequest SIP Invite message to check
     @param required Will return true if the auto-answer is "required"
   */
   virtual bool shouldAutoAnswer(const resip::SipMessage& inviteRequest, bool *required = 0);

   /** 
     Get/Set wether auto answer requests should be digest challenged
     or not.

     @note Requests will be challenged using the digest credentials
           set on this profile
     
     @return bool True if auto answer requests should be challenged
   */
   virtual bool& challengeAutoAnswerRequests() { return mChallengeAutoAnswerRequests; }
   virtual const bool challengeAutoAnswerRequests() const { return mChallengeAutoAnswerRequests; }

   /** 
     Get/Set wether out of dialog REFER requests should be digest 
     challenged or not.

     @note Requests will be challenged using the digest credentials
           set on this profile
     
     @return bool True if OOD REFER requests should be challenged
   */
   virtual bool& challengeOODReferRequests() { return mChallengeOODReferRequests; }
   virtual const bool challengeOODReferRequests() const { return mChallengeOODReferRequests; }

   /** 
     Get/Set the secure media mode that will be used for sending/receiving media packets.
     NoSecureMedia - don't use any secure media strategies - RTP packets are sent 
                     unencrypted via the specified transport.
     Srtp          - use SRTP with keying outside of media stream - ie. SDES key negotiation via SDP (default)
     SrtpDtls      - use SRTP with DTLS key negotiation

     @note If TurnTlsAllocation NatTraversalMode is used, then media will be secured from 
           this UA to the TURN the turn server, even if NoSecureMedia is used.

     @return SecureMediaMode
   */
   virtual SecureMediaMode& secureMediaMode() { return mSecureMediaMode; }
   virtual const SecureMediaMode secureMediaMode() const { return mSecureMediaMode; }

   /** 
     Get/Set the wether Secure Media is required (default is false).
     - if required then SAVP transport protocol is signalled in SDP offers
     - if not required then AVP transport portocol is signalled in SDP offers 
       and encryption=optional attribute is added

     @return true if secure media is required
   */
   virtual bool& secureMediaRequired() { return mSecureMediaRequired; }
   virtual const bool secureMediaRequired() const { return mSecureMediaRequired; }


   /** 
     Get/Set the secure media default crypto suite.  The default crypto suite is used when
     forming SDP offers (SDES only - does not apply to DTLS-SRTP).
     SRTP_AES_CM_128_HMAC_SHA1_32 - Counter Mode AES 128 bit encryption with 
                                    32bit authenication code 
     SRTP_AES_CM_128_HMAC_SHA1_80 - Counter Mode AES 128 bit encryption with 
                                    80bit authenication code (default)

     @return SecureMediaMode
   */
   virtual SecureMediaCryptoSuite& secureMediaDefaultCryptoSuite() { return mDefaultSecureMediaCryptoSuite; }
   virtual const SecureMediaCryptoSuite secureMediaDefaultCryptoSuite() const { return mDefaultSecureMediaCryptoSuite; }

   /** 
     Get/Set the NAT traversal mode that will be used for sending/receiving media packets.
     NoNatTraversal - don't use any NAT traversal strategies - local address and port
                      are used in SDP negotiations
     StunBindDiscovery - use Binding discovery on a STUN server, to discover and use
                         "public" address and port in SDP negotiations
     TurnUdpAllocation - Use a TURN server as a media relay.  Communicate to the TURN
                         server over UDP and Allocate a UDP relay address and port to 
                         use in SDP negotiations
     TurnTcpAllocation - Use a TURN server as a media relay.  Communicate to the TURN
                         server over TCP and Allocate a UDP relay address and port to 
                         use in SDP negotiations
     TurnTlsAllocation - Use a TURN server as a media relay.  Communicate to the TURN
                         server over TLS and Allocate a UDP relay address and port to 
                         use in SDP negotiations
     
     @return NatTraversalMode
   */
   virtual NatTraversalMode& natTraversalMode() { return mNatTraversalMode; }
   virtual const NatTraversalMode natTraversalMode() const { return mNatTraversalMode; }

   /** 
     Get/Set the NAT traversal server hostname that will be used for if natTranversalMode
     is anything other than NoNatTraversal.  The server can be specified as an IP address
     or a A-Records DNS resolvable hostname.  SRV resolution is not yet supported.
     
     @return The server to use for NAT traversal
   */
   virtual resip::Data& natTraversalServerHostname() { return mNatTraversalServerHostname; }
   virtual const resip::Data natTraversalServerHostname() const { return mNatTraversalServerHostname; }

   /** 
     Get/Set the NAT traversal server port that will be used for if natTranversalMode
     is anything other than NoNatTraversal.  
     
     @return The server port to use for NAT traversal
   */
   virtual unsigned short& natTraversalServerPort() { return mNatTraversalServerPort; }
   virtual const unsigned short natTraversalServerPort() const { return mNatTraversalServerPort; }

   /** 
     Get/Set the STUN username that will be used for if natTranversalMode
     is anything other than NoNatTraversal.  The stun username/password is used for 
     both STUN and TURN server authentication.
     
     @return The STUN username
   */
   virtual resip::Data& stunUsername() { return mStunUsername; }
   virtual const resip::Data stunUsername() const { return mStunUsername; }

   /** 
     Get/Set the STUN password that will be used for if natTranversalMode
     is anything other than NoNatTraversal.  The stun username/password is used for 
     both STUN and TURN server authentication.
     
     @return The STUN password
   */
   virtual resip::Data& stunPassword() { return mStunPassword; }
   virtual const resip::Data stunPassword() const { return mStunPassword; }

private:            
   resip::SdpContents mSessionCaps;    
   bool mAllowAutoAnswer;
   bool mAllowPriorityAutoAnswer;
   bool mChallengeAutoAnswerRequests;
   bool mChallengeOODReferRequests;
   SecureMediaMode mSecureMediaMode;
   bool mSecureMediaRequired;
   SecureMediaCryptoSuite mDefaultSecureMediaCryptoSuite;
   NatTraversalMode mNatTraversalMode;
   resip::Data mNatTraversalServerHostname;
   unsigned short mNatTraversalServerPort;
   resip::Data mStunUsername;
   resip::Data mStunPassword;
};

}

#endif


/* ====================================================================

 Copyright (c) 2007-2008, Plantronics, Inc.
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are 
 met:

 1. Redistributions of source code must retain the above copyright 
    notice, this list of conditions and the following disclaimer. 

 2. Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution. 

 3. Neither the name of Plantronics nor the names of its contributors 
    may be used to endorse or promote products derived from this 
    software without specific prior written permission. 

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
 "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
 LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR 
 A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
 OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
 SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
 LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
 DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
 THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 ==================================================================== */
