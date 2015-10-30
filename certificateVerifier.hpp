

#if VMIME_HAVE_TLS_SUPPORT

// Certificate verifier (TLS/SSL)
class interactiveCertificateVerifier : public vmime::security::cert::defaultCertificateVerifier
{
public:

	void verify(vmime::shared_ptr <vmime::security::cert::certificateChain> chain, const vmime::string& hostname)
	{
		try
		{
			setX509TrustedCerts(m_trustedCerts);

			defaultCertificateVerifier::verify(chain, hostname);
		}
		catch (vmime::security::cert::certificateException&)
		{
			// Obtain subject's certificate
			vmime::shared_ptr <vmime::security::cert::certificate> cert = chain->getAt(0);

			std::cout << std::endl;
			std::cout << "Server sent a '" << cert->getType() << "'" << " certificate." << std::endl;

			if (true)
			{
				// Accept it, and remember user's choice for later
				if (cert->getType() == "X.509")
				{
					m_trustedCerts.push_back(vmime::dynamicCast
						<vmime::security::cert::X509Certificate>(cert));

					setX509TrustedCerts(m_trustedCerts);
					defaultCertificateVerifier::verify(chain, hostname);
				}

				return;
			}

			throw vmime::security::cert::certificateException
				("User did not accept the certificate.");
		}
	}

private:

	static std::vector <vmime::shared_ptr <vmime::security::cert::X509Certificate> > m_trustedCerts;
};


std::vector <vmime::shared_ptr <vmime::security::cert::X509Certificate> >
	interactiveCertificateVerifier::m_trustedCerts;

#endif // VMIME_HAVE_TLS_SUPPORT

