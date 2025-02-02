// Copyrights(c) 2017-2019, The Electroneum Project
// Copyrights(c) 2014-2017, The Monero Project
// 
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without modification, are
// permitted provided that the following conditions are met:
// 
// 1. Redistributions of source code must retain the above copyright notice, this list of
//    conditions and the following disclaimer.
// 
// 2. Redistributions in binary form must reproduce the above copyright notice, this list
//    of conditions and the following disclaimer in the documentation and/or other
//    materials provided with the distribution.
// 
// 3. Neither the name of the copyright holder nor the names of its contributors may be
//    used to endorse or promote products derived from this software without specific
//    prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
// THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
// STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
// THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// 
// Parts of this file are originally copyright (c) 2012-2013 The Cryptonote developers

#pragma once

#include <cstddef>
#include <boost/thread/mutex.hpp>
#include <boost/thread/lock_guard.hpp>
#include <boost/archive/iterators/binary_from_base64.hpp>
#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/hex.hpp>
#include <vector>

#include "ed25519-donna/ed25519.h"

#include "common/pod-class.h"
#include "generic-ops.h"
#include "hash.h"

namespace crypto {

  extern "C" {
#include "random.h"
  }

  extern boost::mutex random_lock;

#pragma pack(push, 1)
  POD_CLASS ec_point {
    char data[32];
  };

  POD_CLASS ec_scalar {
    char data[32];
  };

  POD_CLASS public_key: ec_point {
    friend class crypto_ops;
  };

  POD_CLASS secret_key: ec_scalar {
    friend class crypto_ops;
  };

  POD_CLASS public_keyV {
    std::vector<public_key> keys;
    int rows;
  };

  POD_CLASS secret_keyV {
    std::vector<secret_key> keys;
    int rows;
  };

  POD_CLASS public_keyM {
    int cols;
    int rows;
    std::vector<secret_keyV> column_vectors;
  };

  POD_CLASS key_derivation: ec_point {
    friend class crypto_ops;
  };

  POD_CLASS key_image: ec_point {
    friend class crypto_ops;
  };

  POD_CLASS signature {
    ec_scalar c, r;
    friend class crypto_ops;
  };
#pragma pack(pop)

  static_assert(sizeof(ec_point) == 32 && sizeof(ec_scalar) == 32 &&
    sizeof(public_key) == 32 && sizeof(secret_key) == 32 &&
    sizeof(key_derivation) == 32 && sizeof(key_image) == 32 &&
    sizeof(signature) == 64, "Invalid structure size");

  class crypto_ops {
    crypto_ops();
    crypto_ops(const crypto_ops &);
    void operator=(const crypto_ops &);
    ~crypto_ops();

    static secret_key generate_keys(public_key &pub, secret_key &sec, const secret_key& recovery_key = secret_key(), bool recover = false);
    friend secret_key generate_keys(public_key &pub, secret_key &sec, const secret_key& recovery_key, bool recover);
    static bool check_key(const public_key &);
    friend bool check_key(const public_key &);
    static bool secret_key_to_public_key(const secret_key &, public_key &);
    friend bool secret_key_to_public_key(const secret_key &, public_key &);
    static bool generate_key_derivation(const public_key &, const secret_key &, key_derivation &);
    friend bool generate_key_derivation(const public_key &, const secret_key &, key_derivation &);
    static void derivation_to_scalar(const key_derivation &derivation, size_t output_index, ec_scalar &res);
    friend void derivation_to_scalar(const key_derivation &derivation, size_t output_index, ec_scalar &res);
    static bool derive_public_key(const key_derivation &, std::size_t, const public_key &, public_key &);
    friend bool derive_public_key(const key_derivation &, std::size_t, const public_key &, public_key &);
    static void derive_secret_key(const key_derivation &, std::size_t, const secret_key &, secret_key &);
    friend void derive_secret_key(const key_derivation &, std::size_t, const secret_key &, secret_key &);
    static void generate_signature(const hash &, const public_key &, const secret_key &, signature &);
    friend void generate_signature(const hash &, const public_key &, const secret_key &, signature &);
    static bool check_signature(const hash &, const public_key &, const signature &);
    friend bool check_signature(const hash &, const public_key &, const signature &);
    static void generate_tx_proof(const hash &, const public_key &, const public_key &, const public_key &, const secret_key &, signature &);
    friend void generate_tx_proof(const hash &, const public_key &, const public_key &, const public_key &, const secret_key &, signature &);
    static bool check_tx_proof(const hash &, const public_key &, const public_key &, const public_key &, const signature &);
    friend bool check_tx_proof(const hash &, const public_key &, const public_key &, const public_key &, const signature &);
    static void generate_key_image(const public_key &, const secret_key &, key_image &);
    friend void generate_key_image(const public_key &, const secret_key &, key_image &);
    static void generate_ring_signature(const hash &, const key_image &,
      const public_key *const *, std::size_t, const secret_key &, std::size_t, signature *);
    friend void generate_ring_signature(const hash &, const key_image &,
      const public_key *const *, std::size_t, const secret_key &, std::size_t, signature *);
    static bool check_ring_signature(const hash &, const key_image &,
      const public_key *const *, std::size_t, const signature *);
    friend bool check_ring_signature(const hash &, const key_image &,
      const public_key *const *, std::size_t, const signature *);

    static std::string sign_message(const std::string &message, const std::string &privateKey);
    friend std::string sign_message(const std::string &message, const std::string &privateKey);
    static bool verify_signature(const std::string &message, const std::string &publicKey, const std::string &signature);
    friend bool verify_signature(const std::string &message, const std::string &publicKey, const std::string &signature);
    static bool verify_signature(const std::string &message, std::vector<std::string> publicKey, const std::string &signature);
    friend bool verify_signature(const std::string &message, std::vector<std::string> publicKey, const std::string &signature);

    static std::vector<std::string> create_ed25519_keypair();
    friend std::vector<std::string> create_ed25519_keypair();

    static std::string base64_decode(std::string val);
    friend std::string base64_decode(std::string val);
    static std::string base64_encode(std::string val);
    friend std::string base64_encode(std::string val);
  };

  /* Generate N random bytes
   */
  inline void rand(size_t N, uint8_t *bytes) {
    boost::lock_guard<boost::mutex> lock(random_lock);
    generate_random_bytes_not_thread_safe(N, bytes);
  }

  /* Generate a value filled with random bytes.
   */
  template<typename T>
  typename std::enable_if<std::is_pod<T>::value, T>::type rand() {
    typename std::remove_cv<T>::type res;
    boost::lock_guard<boost::mutex> lock(random_lock);
    generate_random_bytes_not_thread_safe(sizeof(T), &res);
    return res;
  }

  /* Generate a new key pair
   */
  inline secret_key generate_keys(public_key &pub, secret_key &sec, const secret_key& recovery_key = secret_key(), bool recover = false) {
    return crypto_ops::generate_keys(pub, sec, recovery_key, recover);
  }

  /* Check a public key. Returns true if it is valid, false otherwise.
   */
  inline bool check_key(const public_key &key) {
    return crypto_ops::check_key(key);
  }

  /* Checks a private key and computes the corresponding public key.
   */
  inline bool secret_key_to_public_key(const secret_key &sec, public_key &pub) {
    return crypto_ops::secret_key_to_public_key(sec, pub);
  }

  /* To generate an ephemeral key used to send money to:
   * * The sender generates a new key pair, which becomes the transaction key. The public transaction key is included in "extra" field.
   * * Both the sender and the receiver generate key derivation from the transaction key, the receivers' "view" key and the output index.
   * * The sender uses key derivation and the receivers' "spend" key to derive an ephemeral public key.
   * * The receiver can either derive the public key (to check that the transaction is addressed to him) or the private key (to spend the money).
   */
  inline bool generate_key_derivation(const public_key &key1, const secret_key &key2, key_derivation &derivation) {
    return crypto_ops::generate_key_derivation(key1, key2, derivation);
  }
  inline bool derive_public_key(const key_derivation &derivation, std::size_t output_index,
    const public_key &base, public_key &derived_key) {
    return crypto_ops::derive_public_key(derivation, output_index, base, derived_key);
  }
  inline void derivation_to_scalar(const key_derivation &derivation, size_t output_index, ec_scalar &res) {
    return crypto_ops::derivation_to_scalar(derivation, output_index, res);
  }
  inline void derive_secret_key(const key_derivation &derivation, std::size_t output_index,
    const secret_key &base, secret_key &derived_key) {
    crypto_ops::derive_secret_key(derivation, output_index, base, derived_key);
  }

  /* Generation and checking of a standard signature.
   */
  inline void generate_signature(const hash &prefix_hash, const public_key &pub, const secret_key &sec, signature &sig) {
    crypto_ops::generate_signature(prefix_hash, pub, sec, sig);
  }
  inline bool check_signature(const hash &prefix_hash, const public_key &pub, const signature &sig) {
    return crypto_ops::check_signature(prefix_hash, pub, sig);
  }

  /* Generation and checking of a tx proof; given a tx pubkey R, the recipient's view pubkey A, and the key 
   * derivation D, the signature proves the knowledge of the tx secret key r such that R=r*G and D=r*A
   */
  inline void generate_tx_proof(const hash &prefix_hash, const public_key &R, const public_key &A, const public_key &D, const secret_key &r, signature &sig) {
    crypto_ops::generate_tx_proof(prefix_hash, R, A, D, r, sig);
  }
  inline bool check_tx_proof(const hash &prefix_hash, const public_key &R, const public_key &A, const public_key &D, const signature &sig) {
    return crypto_ops::check_tx_proof(prefix_hash, R, A, D, sig);
  }

  /* To send money to a key:
   * * The sender generates an ephemeral key and includes it in transaction output.
   * * To spend the money, the receiver generates a key image from it.
   * * Then he selects a bunch of outputs, including the one he spends, and uses them to generate a ring signature.
   * To check the signature, it is necessary to collect all the keys that were used to generate it. To detect double spends, it is necessary to check that each key image is used at most once.
   */
  inline void generate_key_image(const public_key &pub, const secret_key &sec, key_image &image) {
    crypto_ops::generate_key_image(pub, sec, image);
  }
  inline void generate_ring_signature(const hash &prefix_hash, const key_image &image,
    const public_key *const *pubs, std::size_t pubs_count,
    const secret_key &sec, std::size_t sec_index,
    signature *sig) {
    crypto_ops::generate_ring_signature(prefix_hash, image, pubs, pubs_count, sec, sec_index, sig);
  }
  inline bool check_ring_signature(const hash &prefix_hash, const key_image &image,
    const public_key *const *pubs, std::size_t pubs_count,
    const signature *sig) {
    return crypto_ops::check_ring_signature(prefix_hash, image, pubs, pubs_count, sig);
  }

  /* Variants with vector<const public_key *> parameters.
   */
  inline void generate_ring_signature(const hash &prefix_hash, const key_image &image,
    const std::vector<const public_key *> &pubs,
    const secret_key &sec, std::size_t sec_index,
    signature *sig) {
    generate_ring_signature(prefix_hash, image, pubs.data(), pubs.size(), sec, sec_index, sig);
  }
  inline bool check_ring_signature(const hash &prefix_hash, const key_image &image,
    const std::vector<const public_key *> &pubs,
    const signature *sig) {
    return check_ring_signature(prefix_hash, image, pubs.data(), pubs.size(), sig);
  }

  inline std::string sign_message(const std::string &message, const std::string &privateKey) {
    return crypto_ops::sign_message(message, privateKey);
  }

  inline bool verify_signature(const std::string &message, const std::string &publicKey, const std::string &signature) {
    return crypto_ops::verify_signature(message, publicKey, signature);
  }

  inline bool verify_signature(const std::string &message, std::vector<std::string> publicKey, const std::string &signature) {
    return crypto_ops::verify_signature(message, publicKey, signature);
  }

  inline std::vector<std::string> create_ed25519_keypair() {
    return crypto_ops::create_ed25519_keypair();
  }

  inline std::string base64_decode(const std::string &val) {
    using namespace boost::archive::iterators;
    using It = transform_width<binary_from_base64<std::string::const_iterator>, 8, 6>;
    return boost::algorithm::trim_right_copy_if(std::string(It(std::begin(val)), It(std::end(val))), [](char c) {
        return c == '\0';
    });
  }

  inline std::string base64_encode(const std::string &val) {
    using namespace boost::archive::iterators;
    using It = base64_from_binary<transform_width<std::string::const_iterator, 6, 8>>;
    auto tmp = std::string(It(std::begin(val)), It(std::end(val)));
    return tmp.append((3 - val.size() % 3) % 3, '=');
  }
}

CRYPTO_MAKE_HASHABLE(public_key)
CRYPTO_MAKE_HASHABLE(key_image)
CRYPTO_MAKE_COMPARABLE(signature)
