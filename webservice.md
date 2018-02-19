# Fast Updates

Fast updates is the requirement for OpenDNSSEC to present a signed zone
file containing (all) changes submitted to it within a short interval.
At this time OpenDNSSEC is geared towards being efficient for sign quite
large zones, making a trade off between resource usage and efficiently
signing over an extended run-time the server.
The responsiveness of the service, in terms of how long it would take
to make relative small changes in the input to be available as output
of the service was not a major concern as this service would run as a
hidden master in a large set-up.

User demands have however changed in that people expect changes to
be processed relative quickly.  Therefore the system should be more
responsive to changes in the input and provide faster updates.  Be more
dynamic, but this requirement is quite different from requiring Dynamic
DNS, as in RFC2136.  There links between the two; for Dynamic DNS you'd
like changes to be processed quickly, and one way to be able to submit
changes in case of fast updates is by using Dynamic DNS.
Dynamic DNS is about how to enter changes, fast updates is the quick
processing of updates.

The main things to change in OpenDNSSEC is to move from an architecture
where a zone is treated as a whole (even though only part is re-signed)
to only look at the changes that occur.  This has a number of internal
architectural consequences, but also some consequences that affect
software using OpenDNSSEC.

## Submitting changes

OpenDNSSEC already allows for different input adapters, to be able to
read a zone from different type of sources.
Upto OpenDNSSEC 2.1 you could only choose between file or DNS based, which would
only allow you to read a full zone from an input file periodically, or use
the DNS protocol to perform a full zone transfer (AXFR) or incremental
zone transfer (IXFR).  This choice was exclusive in that per zone there
it would be either file based, or DNS based.

Fast updates rules out that using a full zone, since that would require a
full pass over a zone, regardless of which mechanism is used.  Both file
based and the AXFR (full zone transfer) of the DNS adapter should not
be used in the regular process (for bootstrapping or incidental usage
this is still permissable).

At the moment only IXFR would then be possible to be used to achieve
fast updates.  There are however a number of drawbacks in using IXFR
for this usage:
- IXFRs aren't really transactions, it requires the supplier of the data
  to be able to replay requests and hold state to perform them;
- only one IXFR operation can be performed at the same time;
- IXFRs must have a fixed sequential ordering.

Although IXFRs remains a valid way to perform fast updates in use cases,
we also believe that there is a demand for other ways.

Dynamic DNS is a standard mechanism to modify a zone in place.  We
expect the community will expect Dynamic DNS to be available to be used
with OpenDNSSEC but its actual use will be limited.  With Dynamic DNS one
really excludes the use of incoming IXFRs.  Also one has to be careful
that AXFR or file-based re-reading of a zone contains all changes done
using Dynamic DNS and no changes.  Furthermore Dynamic DNS may be a
bit cumberson to be used because the DNS wire protocol is used, which
most back-end systems do not fully support (since DNS is asynchronous
it requires building a request response to validate submission).

We also believe that Dynamic DNS is a relative heavy solution.
With dynamic DNS you can express pre-conditions that must be met and
requires the request to contain the properly specify old entries that
need replacing.  This again requires some state to be administred
(though significantly less than with IXFRs).
It is also a heavy solution for OpenDNSSEC as preconditions require more
internal synchronization of data.

It is more common to have a ReSTfull HTTP interface to perform
transactions.  Therefor we also (next to IXFR and Dynamic DNS) want
to offer a web service that allows you to submit changes to a zone.
We believe that there is room for at least two specific uses;

1) A method where any one complete domain name can be replaced.

2) A method applicable to delegations (zone cuts) where all the records
   applicable to the delegation, including glue from within the zone
   is replaced.

The rationale is that many large zones are operated by TLDs where these
updates are primarily to the many delegation in the zone.  From an end-users
point of view, who is editing its delegation at the registrar, most of the
time have a single view (like a single web-page) on all their data.
Users aren't modifying a single field, but are updating all of their data
but a only a few fields actually happen to change.  The amount of
records per delegation is quite limited and can be submitted as one logical
part.  It is normally treated as one, or a few parts anyway by back-ends
and not as individual DNS records.
It is important for the changes to happen as a single transaction, rather than
one change succeeding, but a change to another record not being applied.

However there will be some other records in the zone that are not delegations.
At least the SOA record it present in the zone, and some other method
is needed to update those.  Especially for TLDs, these updates are sparse
and mostly single record changes.  Hence an interface to update a single
record where the transaction is simple is likely to suffice.

## Delegation changes API

Changing a zone delegation through the restful web service is a complete
replacement of the entire delegation, including all sub-records of the zone.

This is best explained with an example.  Support we have a zone consisting of
amonst others:

    $ORIGIN .nl
    bar.nl               NS  ns.nl.
    example.nl           NS  ns.example.nl.
    example.nl           DS  ...
    ns.example.nl        A   127.0.0.1
    test.test.example.nl TXT "This is a test"
    foo.nl               A   127.0.0.1

Then the restful interface allows you to change the restful object
"example.nl" (which is a delegation).  Any change to this object would
replace all four example.nl records, including the ns.example.nl and
test.test.example.nl.  A possible replacement could be:

    example.nl           NS  ns.nl.

Which will lead to a zone with:

    $ORIGIN .nl
    bar.nl               NS  ns.nl.
    example.nl           NS  ns.nl.
    foo.nl               A   127.0.0.1

Note the other three records are no longer present.

Intoducing a new delegation just involved overwriting a not-yet existing
delegation.  Deleting a delegation involved overwriting an existing
delegation, but providing no replacement records.

Note that this interface is meant for replacing delegations.  Applications
should not use it to replace existing records in which there is no NS or DS
available.
In the same respect, the replacement records should contain a delegation
remove the delegation entirely.

Hence replacing "foo.nl" isn't the right use case for this method, and
trying to replace "bar.nl" with:

        quuz.bar.nl          NS  ns.nl.

Isn't proper either.

Changes are made using the HTTP PUT method to a URL with a certain base
url identifying the major version of the API end-point.  Appended to this
base url is the name of the change we are performing "changedelegation".
Appended to that are the zone name that is targeted and the delegation object
we are replacing:

    PUT /api/v1/changedelegation/nl/example.nl

JSON request to the "example.nl" restful object could be made as such:

    { "apiversion": "20171101",
      "transaction": "change example.nl",
      "entities": [
                { "name": "example.nl.",
                  "type": "NS",
                  "class": "IN"
                  "ttl": "3600",
                  "rdata": "ns.example", },
                { "name": "example.nl",
                  "type": "DS",
                  "class": "IN"
                  "ttl": "3600",
                  "rdata": "29929 5 2 5AD487D2125717B1D229AD067ABBC43578F286B387781A35144B97BC 58F25A3D", },
                { "name": "ns.example",
                  "type": "A",
                  "class": "IN"
                  "rdata": "127.0.0.1", },
              ] }

The apiversion within the JSON request identifies the minor version of the
API.  The transaction field is used in reporting to identify the request and
is a free format string for the interpretation of the client application only.
The entities are the unsorted DNS records that must reside within the
delegation tree of "example.nl".

All DNS records in the JSON entities, must be the same as the delegation
object specified in the URL, or be sub-domain names. 

The API does not provide a GET interface to retrieve the data as would
might be expected with a ReSTful interface.

## Change single record sets API

This API end-point provided a method to replace a full DNS name entry.
It is meant to replace all DNS records of a single DNS name, so if your
zone would include:

    $ORIGIN .nl
    bar.nl               NS    ns.nl.
    example.nl           A     127.0.0.1
    example.nl           A     10.0.0.1
    example.nl           MX    mail.nl.
    test.example.nl      CNAME foo.nl.
    foo.nl               A     127.0.0.1

Then a request would replace only a single name, for example "example.nl".
This means the other names, including "test.example.nl" are not changed in
the same request.  Such a change request would replace all three DNS entries
of "example.nl", two A and one MX record, regardless of which new input
to the zone is given.  So a possible replacement of:

    example.nl           AAAA  ::1

Results in the zone:

    $ORIGIN .nl
    bar.nl               NS    ns.nl.
    example.nl           AAAA  ::1
    test.example.nl      CNAME foo.nl.
    foo.nl               A     127.0.0.1

Changes are submitted to a restfull JSON end point with a base URL appended
with the name of this method: "changename" after which follow the zone-name
and the actual domain name object:

    PUT /api/v1/changename/nl/example.nl

JSON request to the "example.nl" restful object could be made as such:

    { "apiversion": "20171101",
      "transaction": "change example.nl",
      "entities": [
                { "type": "A",
                  "ttl": "3600",
                  "rdata": "127.0.0.1", },
                { "type": "AAAA",
                  "class": "IN"
                  "rdata": "::1", },
              ] }

The type, ttl and rdata fields are the same as with the change delegation
end-point, but the name attribute is now omitted as this is always the
same at the target name.

Deleting a DNS entry can be accomplished by specifying an empty array of
entities.  Creation is just submitting a change to where no object was
already present

## Update strategy and frequency

Each change submitted to the web interface is processed as a transaction,
where the change is verified and carried through before the web service
call returns a HTTP OK status.  This does not mean the request is fully
processed, signed, and written as output.  Just that the change is
guaranteed to be carried out (unless you obstruct OpenDNSSEC normal
operation).  Even in case of other submited changes or even restarts.

The interval in which OpenDNSSEC will re-sign changes and increment
the SOA serial is governed by the Resign parameter in the KASP configuration.
Since this parameter triggers a possible increment in the SOA serial number,
and an increment to the SOA serial number is required by the DNS protocol
to correctly make changes to a zone, this parameter remains th

However before this parameter could not be set very low as a zone could
not be processed that fast.  

OpenDNSSEC would not increment the SOA serial number nor provide a new version
of the zone if there would be no actual changes to the zone file, so this
parameter can be lowered even if no changes are to be expected normally within
that time frame.  A target of 0 is permissible to push through changes as soon
as a current signing process has finished.

## Considerations, limitations, assumptions,..

- OpenDNSSEC will probably start assuming a zone only contains IN
  (internet) class records.  Signing CH class entries isn't practically
  possible and there seems not to be a practical use case to assume other
  classes at the moment.

- It is not possible to set a different Resign interval on a per-zone
  basis.  If you run multiple zones, you need to set the Resign interval
  to be as low as the highest response time you require.  This might mean
  that other zones are updated far more frequent that you would like.
  This may have consequences for example the updates to the SOA serial
  number incrementing faster than expecting and more notifies when using
  the DNS output adapter.

- Decrementing the Resign interval may lead to a much faster incrementing
  of the SOA serial number in case there are indeed changes to the zone.
  Using a datecounter limits the number of updates that can happen per
  day as only two decimal digits are available to increment.  Switching to
  a plain counter (or unix timestamp) should be considered.

- Note that the usage of double quotes in the JSON API requires escaping them.
  This is necessary for instance in HINFO records to separate the two
  components in the name.

## JSON Schema for API endpoint

The base URL encodes a prefix "/api/" and the major version (only) for the
API "/v1.0/".  The normal base URL will be "/api/v1/"

Then follows the view on the data, either allowing you to change an entire
delegation, or to change a entire record set (but leave other record sets
with in the same name untouched).

The next component in the url will be the zone name.
Followed by the domain name within that zone that is being modified.

### Response Codes

Upon success the following return code should be expected:

    HTTP 204 NO CONTENT

Bad requests or if the requests could be be processed could return in error
responses which amongst others include:

    HTTP 400 BAD REQUEST
    HTTP 401 UNAUTHORIZED
    HTTP 405 METHOD NOT ALLOWED
    HTTP 409 CONFLICT
    HTTP 422 UNPROCESSABLE ENTITY

Bad requests, unallowed methods and unprocessable entries are just indications
some parts of the call were incorrect (HTTP method, JSON format, DNS rdata
problems).

A conflict may arrise when multiple changes to the same DNS entry occur at
at the same time (ie. are submitted while another request to the same name
as not provided a return code yet).  This may in many cased be a transient
problem as long as the front end ensures the right order.

### End point for changing an entire delegation

Required HTTP method is "PUT", this call does not allow for the retrieval
of data and can only be used to create/replace or delete an entire delegation.

Request URL:

    PUT /api/v1/changedelegation/nl/example.nl

Request Body:

    {
      "$schema": "http://json-schema.org/schema#",
      "title": "Change Entire Delegation",
      "type": "object",
      "required": [ "apiversion" ],
      "properties": {
        "apiversion": {
          "type": "string",
          "description": "A date format YYYYMMDD version number representing"
                         "the minor version number"
        },
        "transaction": {
          "type": "string",
          "description": "A free format string that is used in logging or "
                         "reporting concerning this change request"
        },
        "entities": {
          "type": "array",
          "items": {
            "type": "object",
            "required": [ "name", "type", "rdata" ],
            "properties": {
              "name": {
                "type": "string",
                "description": "The domain name.  The domain name should not yet "
                               "exist and not a descendant of an existing "
                               "delegation OR should be be a delegation already."
              },
              "type": {
                "type": "string",
                "description": "The record type of the RR."
              },
              "ttl": {
                "type": "number",
                "description": "The Time To Life part of the RR, if not present "
                               "the default TTL for the zone will be used."
              },
              "class": {
                "type": "string",
                "description": "The class part of the RR, only valid value is IN "
                               "at the moment, which is the default value."
              },
              "rdata": {
                "type": "string",
                "description": "The actual value of the DNS entry, depending on "
                               "the type of the record"
              },
          }
        },
      }
    }

### End point for changing one entire name entry

Required HTTP method is "PUT", this call does not allow for the retrieval
of data and can only be used to create/replace or delete an name entry.

Request URL:

    PUT /api/v1/changename/nl/example.nl

Request Body:

    {
      "$schema": "http://json-schema.org/schema#",
      "title": "Change Entire Name Entry",
      "type": "object",
      "required": [ "apiversion", "name" ],
      "properties": {
        "apiversion": {
          "type": "string",
          "description": "A date format YYYYMMDD version number representing"
                         "the minor version number"
        },
        "transaction": {
          "type": "string",
          "description": "A free format string that is used in logging or "
                         "reporting concerning this change request"
        },
        "entities": {
          "type": "array",
          "items": {
            "type": "object",
            "required": [ "type", "rdata" ],
            "properties": {
              "type": {
                "type": "string",
                "description": "The record type of the RR."
              },
              "ttl": {
                "type": "number",
                "description": "The Time To Life part of the RR, if not present "
                               "the default TTL for the zone will be used."
              },
              "class": {
                "type": "string",
                "description": "The class part of the RR, only valid value is IN "
                               "at the moment, which is the default value."
              },
              "rdata": {
                "type": "string",
                "description": "The actual value of the DNS entry, depending on "
                               "the type of the record"
              },
          }
        },
      }
    }

