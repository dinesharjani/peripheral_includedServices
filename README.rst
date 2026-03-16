.. _peripheral_includedServices:

Bluetooth: Peripheral Included Services
#########################

.. contents::
   :local:
   :depth: 2

The peripheral_includedServices is meant as a testing vehicle for nRF Connect's Device Details Client / Server screen in regards to GATT Included / Nested / Secondary Service(s).

Overview
********

You can use the sample, with a Serial Terminal app in parallel, to test whether yout GATT Attribute Table is shown in the correct manner and that the appropriate attribute is being updated on-screen. This is tricky as, the 'unique identifier' of a GATT attribute is not its UUID, meaning multiple Services, included nested / Secondary / Included Services, may reuse the same UUID, confusing Client applications that rely on UUID to differentiate between them. Hence this sample.
