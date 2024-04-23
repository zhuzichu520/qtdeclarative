// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include <qqmlhighlightsupport_p.h>
#include <qqmlsemantictokens_p.h>

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;
using namespace QLspSpecification;
using namespace QQmlJS::Dom;

/*!
\internal
Make a list of enum names to register the supported token
types and modifiers. It is case-sensitive in the protocol
thus we need to lower the first characters of the enum names.
*/
template <typename EnumType>
static QList<QByteArray> enumToByteArray()
{
    QList<QByteArray> result;
    QMetaEnum metaEnum = QMetaEnum::fromType<EnumType>();
    for (auto i = 0; i < metaEnum.keyCount(); ++i) {
        auto &&enumName = QByteArray(metaEnum.key(i));
        enumName.front() = std::tolower(enumName.front());
        result.emplace_back(std::move(enumName));
    }

    return result;
}

static QList<QByteArray> tokenTypesList()
{
    return enumToByteArray<SemanticTokenTypes>();
}

static QList<QByteArray> tokenModifiersList()
{
    return enumToByteArray<SemanticTokenModifiers>();
}

/*!
\internal
A wrapper class that handles the semantic tokens request for a whole file as described in
https://microsoft.github.io/language-server-protocol/specifications/specification-3-16/#semanticTokens_fullRequest
Sends a QLspSpecification::SemanticTokens data as response that is generated for the entire file.
*/
SemanticTokenFullHandler::SemanticTokenFullHandler(QmlLsp::QQmlCodeModel *codeModel)
    : QQmlBaseModule(codeModel)
{
}

void SemanticTokenFullHandler::process(
        QQmlBaseModule<SemanticTokensRequest>::RequestPointerArgument request)
{
    Responses::SemanticTokensResultType result;
    ResponseScopeGuard guard(result, request->m_response);

    if (!request) {
        qCWarning(semanticTokens) << "No semantic token request is available!";
        return;
    }

    const auto doc = m_codeModel->openDocumentByUrl(
            QQmlLSUtils::lspUriToQmlUrl(request->m_parameters.textDocument.uri));
    DomItem file = doc.snapshot.doc.fileObject(GoTo::MostLikely);

    Highlights highlights;
    result = QLspSpecification::SemanticTokens{ std::nullopt, highlights.collectTokens(file) };
    qCDebug(semanticTokens) << "Encoded semantic tokens "
                            << std::get<QLspSpecification::SemanticTokens>(result).data;
}

void SemanticTokenFullHandler::registerHandlers(QLanguageServer *, QLanguageServerProtocol *protocol)
{
    protocol->registerSemanticTokensRequestHandler(getRequestHandler());
}

/*!
\internal
A wrapper class that handles the semantic tokens delta request for a file
https://microsoft.github.io/language-server-protocol/specifications/specification-3-16/#semanticTokens_deltaRequest
Sends either SemanticTokens or SemanticTokensDelta data as response.
This is generally requested when the text document is edited after receiving full highlighting data.
*/
SemanticTokenDeltaHandler::SemanticTokenDeltaHandler(QmlLsp::QQmlCodeModel *codeModel)
    : QQmlBaseModule(codeModel)
{
}

void SemanticTokenDeltaHandler::process(
        QQmlBaseModule<SemanticTokensDeltaRequest>::RequestPointerArgument request)
{
    Responses::SemanticTokensDeltaResultType result;
    ResponseScopeGuard guard(result, request->m_response);

    if (!request) {
        qCWarning(semanticTokens) << "No semantic token request is available!";
        return;
    }
}

void SemanticTokenDeltaHandler::registerHandlers(QLanguageServer *, QLanguageServerProtocol *protocol)
{
    protocol->registerSemanticTokensDeltaRequestHandler(getRequestHandler());
}

/*!
\internal
A wrapper class that handles the semantic tokens range request for a file
https://microsoft.github.io/language-server-protocol/specifications/specification-3-16/#semanticTokens_rangeRequest
Sends a QLspSpecification::SemanticTokens data as response that is generated for a range of file.
*/
SemanticTokenRangeHandler::SemanticTokenRangeHandler(QmlLsp::QQmlCodeModel *codeModel)
    : QQmlBaseModule(codeModel)
{
}

void SemanticTokenRangeHandler::process(
        QQmlBaseModule<SemanticTokensRangeRequest>::RequestPointerArgument request)
{
    Responses::SemanticTokensRangeResultType result;
    ResponseScopeGuard guard(result, request->m_response);

    if (!request) {
        qCWarning(semanticTokens) << "No semantic token request is available!";
        return;
    }
}

void SemanticTokenRangeHandler::registerHandlers(QLanguageServer *, QLanguageServerProtocol *protocol)
{
    protocol->registerSemanticTokensRangeRequestHandler(getRequestHandler());
}

QQmlHighlightSupport::QQmlHighlightSupport(QmlLsp::QQmlCodeModel *codeModel)
    : m_full(codeModel), m_delta(codeModel), m_range(codeModel)
{
}

QString QQmlHighlightSupport::name() const
{
    return "QQmlHighlightSupport"_L1;
}

void QQmlHighlightSupport::registerHandlers(QLanguageServer *server, QLanguageServerProtocol *protocol)
{
    m_full.registerHandlers(server, protocol);
    m_delta.registerHandlers(server, protocol);
    m_range.registerHandlers(server, protocol);
}

void QQmlHighlightSupport::setupCapabilities(
        const QLspSpecification::InitializeParams &,
        QLspSpecification::InitializeResult &serverCapabilities)
{
    QLspSpecification::SemanticTokensOptions options;
    options.range = false;
    options.full = QJsonObject({ { u"delta"_s, false } });
    options.legend.tokenTypes = tokenTypesList();
    options.legend.tokenModifiers = tokenModifiersList();
    serverCapabilities.capabilities.semanticTokensProvider = options;
}

QT_END_NAMESPACE
