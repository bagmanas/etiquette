#include <string>
#include <vector>
#include <vmime/vmime.hpp>

#include "timeoutHandler.hpp"

#include "Message.hpp"
#include "MailBox.hpp"
#include "MailBoxSetting.hpp"
#include "certificateVerifier.hpp"


static vmime::ref <vmime::net::session> g_session
	= vmime::create <vmime::net::session>();

MailBox::MailBox(std::string const & userName, std::string const & userPassword, std::string const & serverAddress, MailBoxSetting const & mailBoxSetting):
	url_(makeUrl_(userName, userPassword, serverAddress)), store_(nullptr), setting_(mailBoxSetting)
{}

vmime::utility::url MailBox::makeUrl_(std::string const & userName, std::string const & userPassword, std::string const & serverAddress)
{
	std::string connectionString = "imaps://" + userName + ":" + userPassword + "@" + serverAddress;
	vmime::string urlString = connectionString;
	vmime::utility::url url(urlString);
	return url;
}
bool MailBox::connect()
{
	//??
	if(!store_)
	{
		makeStore_(url_);
		store_->connect();
	}
	return true;
}

bool MailBox::disconnect()
{
	if(!store_)
	{
		store_->disconnect();
		store_ = NULL;
	}
	return true;
}

void MailBox::makeStore_(vmime::utility::url const & url)
{
	store_ = g_session->getStore(url);
	store_->setProperty("connection.tls", true);
	store_->setProperty("options.need-authentication", true);
	store_->setTimeoutHandlerFactory(vmime::create <timeoutHandlerFactory>());
	store_->setCertificateVerifier(vmime::create <interactiveCertificateVerifier>());
}

std::vector<Message> MailBox::getUnAnswered()
{
	std::vector<Message> messages;
	std::vector< vmime::ref<vmime::net::folder> > folders = store_->getRootFolder()->getFolders(true);
	for (auto folder : folders)
	{
		folder->open(vmime::net::folder::MODE_READ_ONLY);
		if(!setting_.isIgnoredFolder(folder))
		{
			for(size_t i = 1; i <= folder->getMessageCount(); ++i)
			{
				auto msgVmime = folder->getMessage(i);
				folder->fetchMessage(msgVmime, vmime::net::folder::FetchOptions::FETCH_FLAGS  | vmime::net::folder::FetchOptions::FETCH_ENVELOPE);
				Message msg(msgVmime, folder);
				if (msg.isAnswered())
				{
					continue;
				}
				messages.push_back(msg);
			}
		}
	}
	return messages;
	
}

MailBox::~MailBox()
{
	if(store_)
	{
		disconnect();
	}
}